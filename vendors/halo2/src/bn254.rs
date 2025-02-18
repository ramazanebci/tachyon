use std::{
    io::{self, Write},
    marker::PhantomData,
};

use ff::PrimeField;
use halo2_proofs::transcript::{
    Challenge255, EncodedChallenge, Transcript, TranscriptWrite, TranscriptWriterBuffer,
};
use halo2curves::{Coordinates, CurveAffine};

use tachyon_rs::math::elliptic_curves::bn::bn254::{
    Fr as FrImpl, G1JacobianPoint as G1JacobianPointImpl, G1Point2 as G1Point2Impl,
};

pub struct G1MSM;
pub struct G1MSMGpu;
pub struct G1JacobianPoint(pub G1JacobianPointImpl);
pub struct G1Point2(pub G1Point2Impl);
pub struct Fr(pub FrImpl);

#[cxx::bridge(namespace = "tachyon::halo2_api::bn254")]
pub mod ffi {
    extern "Rust" {
        type G1MSM;
        type G1MSMGpu;
        type G1JacobianPoint;
        type G1Point2;
        type Fr;
    }

    unsafe extern "C++" {
        include!("vendors/halo2/include/bn254_msm.h");
        #[cfg(feature = "gpu")]
        include!("vendors/halo2/include/bn254_msm_gpu.h");

        fn create_g1_msm(degree: u8) -> Box<G1MSM>;
        fn destroy_g1_msm(msm: Box<G1MSM>);
        unsafe fn g1_point2_msm(
            msm: *mut G1MSM,
            bases: &[G1Point2],
            scalars: &[Fr],
        ) -> Box<G1JacobianPoint>;
        #[cfg(feature = "gpu")]
        fn create_g1_msm_gpu(degree: u8, algorithm: i32) -> Box<G1MSMGpu>;
        #[cfg(feature = "gpu")]
        fn destroy_g1_msm_gpu(msm: Box<G1MSMGpu>);
        #[cfg(feature = "gpu")]
        unsafe fn g1_point2_msm_gpu(
            msm: *mut G1MSMGpu,
            bases: &[G1Point2],
            scalars: &[Fr],
        ) -> Box<G1JacobianPoint>;
    }

    unsafe extern "C++" {
        include!("vendors/halo2/include/bn254_prover.h");

        fn create_proof(degree: u8);
    }

    unsafe extern "C++" {
        include!("vendors/halo2/include/bn254_blake2b_writer.h");

        type Blake2bWriter;

        fn new_blake2b_writer() -> UniquePtr<Blake2bWriter>;
        fn update(self: Pin<&mut Blake2bWriter>, data: &[u8]);
        fn finalize(self: Pin<&mut Blake2bWriter>, result: &mut [u8; 64]);
    }
}

pub struct Blake2bWrite<W: Write, C: CurveAffine, E: EncodedChallenge<C>> {
    state: cxx::UniquePtr<ffi::Blake2bWriter>,
    writer: W,
    _marker: PhantomData<(W, C, E)>,
}

impl<W: Write, C: CurveAffine> Transcript<C, Challenge255<C>>
    for Blake2bWrite<W, C, Challenge255<C>>
{
    fn squeeze_challenge(&mut self) -> Challenge255<C> {
        // Prefix to a prover's message soliciting a challenge
        const BLAKE2B_PREFIX_CHALLENGE: u8 = 0;
        self.state.pin_mut().update(&[BLAKE2B_PREFIX_CHALLENGE]);
        let mut result: [u8; 64] = [0; 64];
        self.state.pin_mut().finalize(&mut result);
        Challenge255::<C>::new(&result)
    }

    fn common_point(&mut self, point: C) -> io::Result<()> {
        // Prefix to a prover's message containing a curve point
        const BLAKE2B_PREFIX_POINT: u8 = 1;
        self.state.pin_mut().update(&[BLAKE2B_PREFIX_POINT]);
        let coords: Coordinates<C> = Option::from(point.coordinates()).ok_or_else(|| {
            io::Error::new(
                io::ErrorKind::Other,
                "cannot write points at infinity to the transcript",
            )
        })?;
        self.state.pin_mut().update(coords.x().to_repr().as_ref());
        self.state.pin_mut().update(coords.y().to_repr().as_ref());

        Ok(())
    }

    fn common_scalar(&mut self, scalar: C::Scalar) -> io::Result<()> {
        // Prefix to a prover's message containing a scalar
        const BLAKE2B_PREFIX_SCALAR: u8 = 2;
        self.state.pin_mut().update(&[BLAKE2B_PREFIX_SCALAR]);
        self.state.pin_mut().update(scalar.to_repr().as_ref());
        Ok(())
    }
}

impl<W: Write, C: CurveAffine> TranscriptWrite<C, Challenge255<C>>
    for Blake2bWrite<W, C, Challenge255<C>>
{
    fn write_point(&mut self, point: C) -> io::Result<()> {
        self.common_point(point)?;
        let compressed = point.to_bytes();
        self.writer.write_all(compressed.as_ref())
    }

    fn write_scalar(&mut self, scalar: C::Scalar) -> io::Result<()> {
        self.common_scalar(scalar)?;
        let data = scalar.to_repr();
        self.writer.write_all(data.as_ref())
    }
}

impl<W: Write, C: CurveAffine> TranscriptWriterBuffer<W, C, Challenge255<C>>
    for Blake2bWrite<W, C, Challenge255<C>>
{
    /// Initialize a transcript given an output buffer.
    fn init(writer: W) -> Self {
        Blake2bWrite {
            state: ffi::new_blake2b_writer(),
            writer: writer,
            _marker: PhantomData,
        }
    }

    fn finalize(self) -> W {
        // TODO: handle outstanding scalars? see issue #138
        self.writer
    }
}
