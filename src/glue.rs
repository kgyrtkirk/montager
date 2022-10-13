use opencv::prelude::Mat;

use shape_core::*;
// use shape_core;
// use shape_core::;
use projective::Projective;
// use 
use num_traits::real::Real;

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

fn fx(){
    let mut p : Point<f64> = Point::new(1.0f64,0.0f64);
    // let x : Projective = p;
    // let m = Mat::from_slice_2d(&[ [1,0,10],[0,1,10],[0,0,1]])?;
    // p.translate_x(1.0f64);
    // use shape_core::Point;
    
    p.translate_x(&1.0f64);
    // println!("{:#}", Point::new(100, 100).tra);


}
#[cfg(test)]
mod tests {
            // Note this useful idiom: importing names from outer (for mod tests) scope.
    use super::*;

    #[test]
    fn testConversion() {
        let x = X { x: 1, y: 2 };
        let v : i32 = x.into();
        assert_eq!(v,3);
    }

    #[test]
    fn testMatToProjective() {
        fx();

        // let x = X { x: 1, y: 2 };
        // let v : i32 = x.into();
        // assert_eq!(v,3);
    }
}
