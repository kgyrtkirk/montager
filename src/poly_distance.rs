use opencv::core::*;
use opencv::types::{VectorOfPoint2d, VectorOfPoint};
use opencv::*;


// FIXME: this is ugly; might be better to do this differently?
pub trait F64I32Bridge<B> {
    fn to_f64(self : Self) -> B;
}

impl F64I32Bridge<Point2d> for Point {
    fn to_f64(self : Self) -> Point2d {
        Point2d::new(self.x as f64, self.y as f64)
    }
}

impl F64I32Bridge<Point2i> for Point2d {
    fn to_f64(self : Self) -> Point2i {
        Point2i::new(self.x as i32, self.y as i32)
    }
}

// FIXME: I can't template this yet...
impl F64I32Bridge<VectorOfPoint> for VectorOfPoint2d {
    fn to_f64(self : Self) -> VectorOfPoint {
        let mut ret :VectorOfPoint = Vector::new();
        for p in self {
            ret.push(p.to_f64());
        }
        ret
    }
}

pub trait PolyDist {
    fn dist(&self, p: &Point_<f64>) -> Result<f64>;
}

// FIXME: template?
impl PolyDist for core::Point_<f64> {
    fn dist(&self, p: &Point_<f64>) -> Result<f64> {
        let v2 = *self - *p;
        Ok(v2.norm())
    }
}

trait AbsMin {
    fn abs_min(&mut self, o: &Self);
}

impl AbsMin for f64 {
    fn abs_min(&mut self, o: &Self) {
        if self.abs() > o.abs() {
            *self = *o;
        }
    }
}

pub trait Transform  {
    fn transform(&mut self, m: &Mat) ;
}

impl Transform for Point2d {
    fn transform(&mut self, m: &Mat) {
        // let a=Vec3d::from([pos.x as f64, pos.y as f64, 1.0]);
        // let p2=a.to_mat().unwrap();
        let p2 = Mat::from_slice(&[self.x as f64, self.y as f64, 1.0]).unwrap().t().unwrap();
        let r = (m * p2).into_result().unwrap().to_mat().unwrap();
        let a: Point2i = Point2i::new(*r.at::<f64>(0).unwrap() as i32, *r.at::<f64>(1).unwrap() as i32);
        *self=Point2d::new(a.x as f64 , a.y as f64);
    }
}

impl Transform for VectorOfPoint2d {

    fn transform(&mut self, m: &Mat) {

        for p in self.as_mut_slice() {
            p.transform(m);
        }
        // // let a=Vec3d::from([pos.x as f64, pos.y as f64, 1.0]);
        // // let p2=a.to_mat().unwrap();
        // let p2 = Mat::from_slice(&[pos.x as f64, pos.y as f64, 1.0]).unwrap().t().unwrap();
        // let r = (m * p2).into_result().unwrap().to_mat().unwrap();
        // let a: Point2i = Point2i::new(*r.at::<f64>(0).unwrap() as i32, *r.at::<f64>(1).unwrap() as i32);
        // a
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
                dist.abs_min( &n.cross(v));
            } else {
                if vl <= 0.0f64 {
                    dist.abs_min( &p.dist(&p1)?);
                } else {
                    dist.abs_min( &p.dist(&p2)?);
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
