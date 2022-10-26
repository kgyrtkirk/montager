
# Montager - montage composition generator

Originally I've used a manual approach to create montages for our walls; the process was not just time consuming - but it left quite a lot of space unused between the pieces

This project was created to make the process easier and possibly create better montages - with more space-effective layouts and possibly more interesting borders between the images.

## History

* originally I started this project in C++ / OpenCV - to somewhat dust off my c++ knowledge and do something usefull
* then I started to be annoyed with (the ususal) C++ dowhills...and experimented with using Rust instead
* using Rust + some OpenCV lib was okayish...I learned a lot about Rust in the process - however at some point; right around it became clear that I'll also need the ability to move the image parts around/etc - I realized that if I really want to have something usefull; I'll have to change direction - or I will end up writing yet-another-image editing software...
* so I experimented with writing a Gimp plugin instead...
  * lost quite some time because I missed the importance of the `MAIN() ` macro call in the sample plugins. 
  * but eventually had it working...
* messed around with the gimp plugin api to see how I can read layer masks; make changes to it
* tuned the project and made C++ available as I really didn't want to write plain C
* dropped OpenCV as I haven't really feeled that I've profited from using it...(beyond the hull detection algo) - and since now I had much cleared view on what I will most probably need:
  * convex hull algorithm - I can stich together one; but it would be nice not wasting time on such a common piece
  * polygon/point distance stuff - boring stuff; better to have it ready
    * actually in Rust I already had to implement some part of this manually - and I've feeled that it was partially a waste of time
* I've found boost::geometry ; which met my requirements...
* stiched together some crude stuff to compute the convex hull of the full opacity layer mask parts
* read some paper about polygon distance calculation methods - I thinked that might be the bottleneck later...but digging into the relating CS stuff helped to tune my mind for the topic a bit better
  * I have seen Voronoi diagrams before; but it was obvious now that I'm making a similar grouping - but instead of just a single point as generators I have a set of points
  * the approach the paper took seemed pretty logical - and although I just casually read the algo part; I feeled that the biggest distinction of mine and the general distance problem is that I'll have to scan a full 2D grid; so I might find optimization opportunities from that direction
* eventually made some top level model to describe the montage...and the possibility to write some cruel assignment method
* I was using some small gimp file as quick reality check on how this will work...
* I have found an earlier montage's xcf - and I've preared it to be used with the plugin - to have a "real life" problem
  * some of the layers had their opacity burned in for some reason - I had to ignore those...but luckily there were quite a few layers I decided earlier to not use ; enabled some of them - and made a crude arrangement
  * the image is around 8000x5000 with ~30 images
* I really wanted to get a glimpse on how the end result will look like; and since I had to do other activities I just started it...
  * after 4 hours I've seen that the progress bar was at around 25%; so I killed it....this is way too slow!
* I was doing `width*height*nImage` distance measurements...
* I came up with an idea of employing some simple algo to reduce the number of distance measurements to find the region the point belongs to
  * idea was to
* this got me to pretty reasonable execution times with the small test image; and to ~30 minutes with the "real life" image
* I wanted to have this execution time below at least making a cup of coffee...so 3-5 minutes at tops...
* My next idea was to exploit the locality properties of the problem:
  * if some point belong to group A - then most likely the neighbouring ones also belong there
  * this have lead me to compute the `min_radius` - and assign that entire circle to some region
* this have got me down to almost-instant voronoi gen on the small and ~59s on the bigger one
* compared to the other stuff; convex hull started to took significant time - not adding internal points in every row made it almost invisible
* the are filling approach could be the most effective when there is no neighbour pixels assigned to anywhere...
  * found a typo; which resulted in filling to small places; this got me around 36s
  * tried to process the hull centroids first - in the hope that they will cover a large blob - but the gain was negligable; so I've throw it out
  * tried some hilber curves - but that wasn't beneficial either; so I've removed that one as well

testrun1:	8000x5000	~30 image
18:05 ~ 22:00 ~ 25%

testrun2; +dist_queue
14:59 ~ 15:27	
