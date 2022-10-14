use opencv::core::*;
use opencv::types::VectorOfPoint2d;
use opencv::*;

trait PolyDist {
    fn dist(&self, p: &Point_<f64>) -> Result<f64>;
}

// FIXME: template?
impl PolyDist for core::Point_<f64> {
    fn dist(&self, p: &Point_<f64>) -> Result<f64> {
        let v2 = *self - *p;
        Ok(v2.norm())
    }
}

// FIXME: signed dist is only valid for convex polys
impl PolyDist for VectorOfPoint2d {
    fn dist(&self, p: &Point_<f64>) -> Result<f64> {
        let mut dist = f64::MAX;
        for i in 0..self.len() {
            let p1 = self.get(i + 0)?;
            let p2 = self.get((i + 1) % self.len())?;

            let mut n = p2 - p1;
            let l = n.norm();
            n /= l;
            let v = *p - p1;
            let vl = v.dot(n);
            if 0.0f64 <= vl && vl <= l {
                let r = n.cross(v);
                if dist.abs() > r.abs() {
                    dist = r;
                }
            } else {
                if vl <= 0.0f64 {
                    let r = p.dist(&p1)?;
                    if dist.abs() > r.abs() {
                        dist = r;
                    }
                } else {
                    let r = p.dist(&p2)?;
                    if dist.abs() > r.abs() {
                        dist = r;
                    }
                }
            }
        }
        Ok(dist)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_point_dist() {
        let p1 = Point2d::new(1.0f64, 1.0f64);
        let p2 = Point2d::new(2.0f64, 2.0f64);
        let d = p1.dist(&p2).unwrap();
        assert_eq!(d, 2.0f64.sqrt())
    }

    /*
     *  2   *
     *  |  /|
     *  1 *-*
     *  |
     *  +-1-2
     */
    fn sample_poly() -> Vector<Point_<f64>> {
        let p1 = Point2d::new(1.0f64, 1.0f64);
        let p2 = Point2d::new(2.0f64, 2.0f64);
        let p3 = Point2d::new(2.0f64, 1.0f64);
        let poly = Vector::from_slice(&[p1, p2, p3]);
        poly
    }

    #[test]
    fn test_poly_dist_on() {
        let poly = sample_poly();

        assert_abs_diff_eq!(poly.dist(&sample_poly().get(2).unwrap()).unwrap(), 0.0f64);
        assert_abs_diff_eq!(poly.dist(&sample_poly().get(0).unwrap()).unwrap(), 0.0f64);
        assert_abs_diff_eq!(poly.dist(&sample_poly().get(1).unwrap()).unwrap(), 0.0f64);
    }
    #[test]
    fn test_poly_dist_left_right_in_range() {
        let poly = sample_poly();
        assert_abs_diff_eq!(
            poly.dist(&Point2d::new(1.6f64, 1.5f64)).unwrap(),
            2.0f64.sqrt() / -20.0f64
        );
        assert_abs_diff_eq!(
            poly.dist(&Point2d::new(1.0f64, 2.0f64)).unwrap(),
            2.0f64.sqrt() / 2.0f64
        );
    }
    #[test]
    fn test_poly_dist_outside_line_end() {
        let poly = sample_poly();
        assert_abs_diff_eq!(
            poly.dist(&Point2d::new(0.0f64, 0.0f64)).unwrap(),
            2.0f64.sqrt()
        );
    }
}
