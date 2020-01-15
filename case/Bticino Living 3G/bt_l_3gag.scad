//use<threads.scad>
//use <096Oled.scad>;
include <NopSCADlib/lib.scad>
use <threadlib/threadlib.scad>
use <OLED_13_128_64.scad>

include <096OledDim.scad>;


renderNop = false;
nutmount = false;

$fn=60;

union(){

    difference(){
        translate([0,0,-0.8])extBox([43.3,66.0,58.8]);
        translate([0.1,0,1])extBox([41.0,63,58.8]);
        translate([39.5,0,19]) cube([4,66.0,48.8]);
        translate([39.5,0,19]) rounded_rectangle([4,66.0,48.8],r=1,center=false);
        translate([1.2,2.0,18.8]) cube([43.0,62.0,40]);
        translate([20,67,33]) rotate([90,90,0]) tearslot(68.2, 10,20, center = false, truncate = true);
  
        
        // OLED
        rotate([0,180,0]){
            translate([-23.5,19.9,-4]) {
              rounded_rectangle([18.8,31.8,5],r=1,center=false);
            }
        }
 //       rotate([180,0,90])
 //         translate([20,25,-2]) {
 //           OLED_13_128_64_I2C(1,0);  
 //           DisplayLocalize(type=I2CSPI7, align=2, dalign=1)
 //               translate([0,0,6.0/2])
 //                   cube([CRIUS_LVW,CRIUS_LVL,6.0], center=true);
 //         }
        // Enc
        rotate([180,0,180])  
          translate([-28.05, 55,-2])
            cylinder(h=5,r=3.8);
          
          
/*        
       translate([22,10,0.5])
          rotate([0,0,-90])
          scale([0.1, 0.1, 0.1])
         surface(file = "lamp.png", center = true, invert = true);
        
       translate([22,32,0.5])
          rotate([0,0,-90])
          scale([0.1, 0.1, 0.1])
         surface(file = "lamp2.png", center = true, invert = true);

       translate([22,55,0.5])
          rotate([0,0,-90])
          scale([0.1, 0.1, 0.1])
         surface(file = "lamp2.png", center = true, invert = true);
 */  
       
        
        // Buco bottone Reset
        translate([2,18,-2])cylinder(r=1,h=28,$fn=60);
        translate([2,11,-2])cylinder(r=1,h=28,$fn=60);
        //translate([39,17,0]) linear_extrude(height=2)  rotate([180,0,90]) text("Connesso", font = "Liberation Sans",  size = 1.3);
        // Buco DHT22
        translate([12,50.9,-2]) rounded_rectangle([16,21,5],r=0.5,center=false);
        translate([6,61+2,1.7]) cube([12,5,2]);
        // Buco fissaggio OLED
        translate([4,-2,2]) cube([35,5,2]);
        

 
    }
//        translate([3.5,40.5,-2]) cube([16,22,5]);

     rotate([0,180,0])
      translate([-40,3,-3])  {
        translate([33,32,0]) m2hold();  
        translate([33,32-30,0]) m2hold();  
        translate([32-27,32-30,0]) m2hold();  
        translate([32-27,32,0]) m2hold();  
//        translate([32-27,32,0]) nut("M2",turns=5,Douter=4.2);  
      }

    if(nutmount){
         translate([33.2,46,0.5]){
            nut("M2",turns=26,Douter=6); 
            translate([-7,0,4.7]) cube([12,4,12],center=true);
            translate([5.5,0,4.7]) cube([9,4,12],center=true);
         }
     }
      
    if(renderNop){
        rotate([0,180,0])
          translate([-39,3,-4]) {
 //           DisplayModule(type=I2CSPI7, align=2, G_COLORS=true); 
            OLED_13_128_64_I2C(1,0);  
   
          }
        rotate([180,0,180])  
          translate([-163,207,-14])
            import("../Rotary_Encorder_KY-040.stl");   
          
        translate([19,61,3.5]) rotate([90,180,0]) import("../DHT22.stl");
         
        translate([27,55,-15]) import("../Rotary_Encoder_knob.stl"); 
    }
    
    PERF60x40_1 = ["PERF60x40", "Perfboard 60 x 40mm"
       , 60
       , 40
       , 1.6
       , 0
       , 2.3
       , 0
       , "green"
       , true
       , 
       [ 
         [1.9,1.9],
         [-1.9,1.9],
         [1.9,-1.9],
         [-1.9,-1.9]
       ], [], [], [5.87, 3.49]];
       
      rotate([90,0,90]) translate([33,37,10]){
          if(renderNop)pcb(PERF60x40_1);
          pcb_screw_positions(PERF60x40_1){
            translate([0,0,-9.5]){
               if(nutmount) nut("M2",turns=20,Douter=6); 
            } 
          }
      }

    
    
    
    // Supporti Scheda
 //   translate([3+1.9,   7,2]) threadSupport();
 //   translate([3+1.9,   53+7,2]) threadSupport();
 //   translate([38+0.6,  53+7,2]) threadSupport();
 //   translate([38+0.6,  7,2]) threadSupport();
 //   %translate([2,3,10]) cube([40,60.2,1.5]);
 
    // Ganci Living
   translate([0,16.9,18.8-8.6])  rotate([90,0,270]) hookMount();

   translate([43.3,16.9-11.5,18.8-8.6])  rotate([90,0,-270]) hookMount();

   translate([0,16.9+21.7,18.8-8.6])  rotate([90,0,270]) hookMount();

   translate([43.3,16.9-11.5+21.7,18.8-8.6])  rotate([90,0,-270]) hookMount();

   translate([0,16.9+(21.7*2),18.8-8.6])  rotate([90,0,270]) hookMount();

   translate([43.3,16.9-11.5+(21.7*2),18.8-8.6])  rotate([90,0,-270]) hookMount();
    
    
}

module extBox(dim){
    translate([21.6,33,29.4]) rotate([90,90,0]) rounded_rectangle([dim.z,dim.x,dim.y],r=4,center=true);
//    intersection(){
//      cube(dim);
//       translate([43.5/2,66,58]) 
//            rotate([90,0,0]) 
//                cylinder(r1=60,r2=60,h=66,$fn=260);
//
//    }

}

module threadSupport(){
    difference () {
        cylinder (r=3, h=5, $fn=100);
     metric_thread (diameter=2, pitch=1, length=6, internal=true, n_starts=6);
    }
}


module hookMount(){
    union(){
        difference(){
            cube([11.8,8.6,3-1.2]);
            translate([(11.8-9.2)/2,1.9,0]) cube([9.2,8.4,3-1.2]);
            translate([3.3,0,0]) cube([4.9,8.6,3-1.2]);
        }
        CubePoints = [
          [  3.3,  5.4,  0 ],  // 0
          [  3.3+4.9,  5.4,  0 ],  // 1
          [  3.3+4.8,  8.6,  0 ],  // 2
          [  3.3, 8.5, 0 ], // 3
          [  3.3,  5.4,  3-1.2 ],  //4
          [  3.3+4.9,  5.4,  3-1.2 ], //5 
          [  3.3+4.9,  10.1,  3-1.2 ],  //6
          [  3.3,  10.1,  3-1.2 ],  //7
         ]; 
  
        CubeFaces = [
          [0,1,2,3],  // bottom
          [4,5,1,0],  // front
          [7,6,5,4],  // top
          [5,6,2,1],  // right
          [6,7,3,2],  // back
          [7,4,0,3]]; // left
          
        polyhedron( CubePoints, CubeFaces );
    }
    
    
}

module m2hold() {
    translate([0,0,-2.2]){
      difference(){ 
        union(){   
           cylinder(r1=1.5,r2=3.9/2,h=2);
           cylinder(r=1.5,h=6);
        }
        cube([4,0.8,7],center=true);
 //       cube([0.8,4,7],center=true);
      }
    }
}
