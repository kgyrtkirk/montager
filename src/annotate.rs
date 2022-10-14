use std::sync::{Arc, Mutex};

use opencv::{
    core::*,
    highgui::{self, imshow, resize_window, EVENT_LBUTTONDOWN, WINDOW_GUI_EXPANDED},
    imgcodecs,
    imgproc::{self, convex_hull, LINE_8},
    // prelude::*,
    types::{VectorOfPoint, VectorOfPoint2d},
//    Result,
};

use color_eyre::Result;
use serde::{Deserialize, Serialize};
use serde_yaml::{self};

use crate::poly_distance::Transform;
use crate::poly_distance::PolyDist;


#[allow(unused)]
#[derive(Debug, Serialize, Deserialize)]
struct OnDiskAnnotations {
    points: Vec<(i32, i32)>,
}

pub struct AnnotationEditor {
    points: VectorOfPoint,
    annotation_file: String,
    #[allow(unused)]
    file_name: String,
    pub image: Mat,
}

#[allow(unused)]
impl AnnotationEditor {
    pub fn new(file_name: &String) -> AnnotationEditor {
        let mut image = imgcodecs::imread(&file_name, 1).unwrap();
        // FIXME: missing image not detected

        println!("asd {}", image.size().unwrap().width);

        let mut points = VectorOfPoint::new();

        let mut annotation_file = file_name.clone();
        annotation_file.push_str(".annot");

        if let Ok(f) = std::fs::File::open(&annotation_file) {
            let data: OnDiskAnnotations =
                serde_yaml::from_reader(f).expect("Could not read values.");
            for (x, y) in data.points {
                points.push(Point2i::new(x, y));
            }
        } else {
            dbg!("annotation file doesn't exists yet");
        }

        AnnotationEditor {
            points: points,
            annotation_file: annotation_file,
            file_name: file_name.clone(),
            image: image,
        }
    }
    fn save(self: &AnnotationEditor) {
        let f =
            std::fs::File::create(&self.annotation_file).expect("Can't open annot file for write");
        let a_points = self.points.iter().map(|p| (p.x, p.y)).collect();
        let ann: OnDiskAnnotations = OnDiskAnnotations { points: a_points };

        serde_yaml::to_writer(f, &ann).unwrap();
        dbg!("annot written");
    }
    fn hull(&self) -> Result<Vector<Point_<f64>>> {
        let points = &self.points;
        if points.len() >= 3 {
            let mut hull_points = VectorOfPoint::new();
            convex_hull(points, &mut hull_points, true, true).unwrap();
            let mut hull_points2 = VectorOfPoint2d::new();
            for p in hull_points {
                hull_points2.push(Point2d::new(p.x as f64,p.y as f64));
            }

            Ok(hull_points2)
        } else {
            let size = self.image.size()?;
            let mut hull_points = VectorOfPoint2d::from_slice(&[
                Point2d::new(0.0f64, 0.0f64),
                Point2d::new(size.width as f64, 0.0f64),
                Point2d::new(size.width as f64, size.height as f64),
                Point2d::new(0.0f64, size.height as f64),
            ]);
            Ok(hull_points)
        }
    }

    fn draw(self: &AnnotationEditor) -> Result<Mat> {
        let mut frame = self.image.clone();
        let points = &self.points;
        if points.len() > 0 {
            let hull_points = self.hull()?;

            for i in 1..points.len() {
                let color = Scalar::new(255., 0., 255., 0.);
                imgproc::line(
                    &mut frame,
                    points.get(i - 1)?,
                    points.get(i)?,
                    color,
                    2,
                    LINE_8,
                    0,
                )?;
            }
            // for i in 0..hull_points.len() {
            //     let color = Scalar::new(0., 255., 0., 0.);
            //     let j = if i > 0 { i - 1 } else { hull_points.len() - 1 };
            //     imgproc::line(
            //         &mut frame,
            //         hull_points.get(j)?,
            //         hull_points.get(i)?,
            //         color,
            //         2,
            //         LINE_8,
            //         0,
            //     )?;
            // }
        }
        Ok(frame)
    }

    fn add_point(self: &mut AnnotationEditor, point: Point2i) {
        self.points.push(point);
    }

    pub(crate) fn make_dist_map(&self, m: Mat, size: Size_<i32>, pos: Point_<i32>) -> Result<Mat> {
        let mut dist_map = Mat::zeros_size(size, CV_64F).unwrap().to_mat().unwrap();

        let mut pp : VectorOfPoint2d=self.hull()?;
        pp.map_point(&m);

        for row in 0..size.height {
            for col in 0..size.width {
                let p = Point2d::new(col as f64, row as f64);
                let p1 = Point2i::new(col, row);
                let p2 = Point2i::new(col, row);
               let d = (p1 - pos).norm();
                let mut d=pp.dist(&p).unwrap();
                d=d.max(0.0f64);
                *dist_map.at_2d_mut::<f64>(row, col).unwrap() = d;//(d - 100.0).max(0.1)
            }
        }
        Ok(dist_map)
    }
}

pub fn editor(file_name: &String) -> Result<()> {
    let annotation_editor = Arc::new(Mutex::new(AnnotationEditor::new(file_name)));
    let window = "montage-gen annotate";

    highgui::named_window(window, highgui::WINDOW_KEEPRATIO | WINDOW_GUI_EXPANDED)?;
    highgui::set_mouse_callback(
        window,
        Some(Box::new({
            let annotation_editor = Arc::clone(&annotation_editor);
            move |event, x, y, _flags| match event {
                EVENT_LBUTTONDOWN => {
                    println!(" {} {} ", x, y);
                    annotation_editor
                        .lock()
                        .unwrap()
                        .add_point(Point2i::new(x, y));
                }
                _ => {}
            }
        })),
    )?;

    resize_window(window, 500, 500)?;
    loop {
        let key = highgui::wait_key(100)?;
        if key == 27 {
            break;
        }
        let annotation_editor = annotation_editor.lock().unwrap();
        if key == 's' as i32 {
            annotation_editor.save();
            break;
        }
        let frame = annotation_editor.draw()?;
        imshow(window, &frame)?;
    }
    Ok(())
}
