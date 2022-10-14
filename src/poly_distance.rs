use std::clone::Clone;
use std::ops::Sub;
use std::ops::SubAssign;
// use std::markers::Copy;

use opencv::core::*;
use opencv::types::VectorOfPoint2d;
use opencv::*;
use opencv::{prelude::Mat, types::VectorOfPoint};

trait PolyDist {
    fn dist(&self, p: &Point_<f64>) -> Result<f64>;
}

// FIXME: template
impl PolyDist for core::Point_<f64> {
    fn dist(&self, p: &Point_<f64>) -> Result<f64> {
        let p1 = p.clone();
        let p2 = self.clone();
        let v = p1 - p2;

        let v2 = *self - *p;

        Ok(v2.norm())
    }
}

impl PolyDist for VectorOfPoint2d {
    fn dist(&self, p: &Point_<f64>) -> Result<f64> {
        let mut dist = f64::MAX;
        for i in 0..self.len() - 1 {
            let p1 = self.get(i + 0)?;
            let p2 = self.get(i + 1)?;

            let mut n = p2 - p1;
            n /= n.norm();

            let r = n.cross(*p);

            if dist.abs() > r.abs() {
                dist = r;
            }
        }
        Ok(dist)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn testPointDist() {
        let p1 = Point2d::new(1.0f64, 1.0f64);
        let p2 = Point2d::new(2.0f64, 2.0f64);
        let d = p1.dist(&p2).unwrap();
        assert_eq!(d, 2.0f64.sqrt())
    }

    #[test]
    fn testPolyDist2() {
        let p1 = Point2d::new(1.0f64, 1.0f64);
        let p2 = Point2d::new(2.0f64, 2.0f64);
        let poly = Vector::from_slice(&[p1, p2]);
        assert_eq!(poly.dist(&p1).unwrap(), 0.0f64.sqrt());
        assert_eq!(poly.dist(&p2).unwrap(), 0.0f64.sqrt());
        assert_abs_diff_eq!(
            poly.dist(&Point2d::new(2.0f64, 1.0f64)).unwrap(),
            2.0f64.sqrt() / -2.0f64
        );
        assert_abs_diff_eq!(
            poly.dist(&Point2d::new(1.0f64, 2.0f64)).unwrap(),
            2.0f64.sqrt() / 2.0f64
        );
    }
}
