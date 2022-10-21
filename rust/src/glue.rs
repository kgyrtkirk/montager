
use shape_core::*;
// use shape_core;
// use shape_core::;
use projective::Projective;
// use 
// use num_traits::real::Real;

#[derive(Debug)]
#[allow(unused)]
struct X {
    x : i32,
    y: i32,
}

impl Into<i32> for X {
    fn into(self) -> i32 {
        self.x+self.y
    }
}

// impl From<Point<f64>> for i32 {
//     fn from(val: Point<f64>) -> Self {
//         todo!()
//     }
// }

#[allow(unused)]
fn fx(){
    let mut p : Point<f64> = Point::new(1.0f64,0.0f64);
    let mut p2 : Point<f64> = Point::new(2.0f64,1.0f64);
    let d=p.distance_to(&p2);
    
    p.translate_x(&1.0f64);
}


#[allow(unused)]
fn fx2(){
    let mut p1 : Point<f64> = Point::new(1.0f64,0.0f64);
    let mut p2 : Point<f64> = Point::new(2.0f64,1.0f64);
    let mut p3 : Point<f64> = Point::new(2.0f64,1.0f64);
    let points = vec![p1,p2,p3];
    let mut p  = Polygon::new(points);
    let mut p2 : Point<f64> = Point::new(2.0f64,1.0f64);


    
    // let d=p.distance_to(&p2);
    
    p.translate_x(&1.0f64);
}


#[cfg(test)]
mod tests {
            // Note this useful idiom: importing names from outer (for mod tests) scope.
    use super::*;

    #[test]
    fn test_conversion() {
        let x = X { x: 1, y: 2 };
        let v : i32 = x.into();
        assert_eq!(v,3);
    }

    #[test]
    fn test_mat_to_projective() {
        fx();

        // let x = X { x: 1, y: 2 };
        // let v : i32 = x.into();
        // assert_eq!(v,3);
    }
    #[test]
    fn test_mat_to_projective2() {
        fx2();
    }
}
