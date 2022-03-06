include <../scad-utils/morphology.scad>
include <./flesnel_lens.scad>
include <./lcd_096in_st7735.scad>

$fn=50;

// EEE_BODY=[LENS_BODY.x + 2.5, LENS_BODY.y + 2.5, 30];
EEE_LCD_MOUNT_Z=2;
EEE_LCD_COVER_MOUNT_Z=EEE_LCD_MOUNT_Z + LCD_MODULE_TOTAL_Z;
EEE_BODY=[LENS_BODY.x + 5, LENS_BODY.y + 5, LENS_TO_DISPLAY + LCD_MODULE_TOTAL_Z + EEE_LCD_MOUNT_Z];
EEE_BODY_FRONT_Z=0.6;

EEE_LEG=[10, 4, 6];

EEE_LID=[EEE_BODY.x - 3, EEE_BODY.y - 3, 1];
EEE_LID_BOTTOM = [EEE_BODY.x, 5, EEE_BODY.z];

EEE_LID_MOUNT_BACK = [5, 10, 5];
EEE_LID_MOUNT_BOTTOM = [2, 7, 7];

module eee_body_base() {
  difference() {
    linear_extrude(height=EEE_BODY.z, center=!true, convexity=10, twist=0)
    rounding(r=3)
    square(size=[EEE_BODY.x, EEE_BODY.y], center=true);

    // inner space
    difference() {
      translate([0, 0, EEE_BODY_FRONT_Z]) 
      linear_extrude(height=EEE_BODY.z - EEE_BODY_FRONT_Z - EEE_LID.z + 0.01, center=!true, convexity=10, twist=0)
      rounding(r=3)
      inset(d=2)
      square(size=[EEE_BODY.x, EEE_BODY.y], center=true);

      eee_lid_overhang_support_deg = 15;

      // support for overhang
      translate([0, -EEE_BODY.y/2 + EEE_LID_BOTTOM.y, EEE_BODY_FRONT_Z]) 
      rotate([-eee_lid_overhang_support_deg, 0, 0]) 
      translate([0, -EEE_LID_BOTTOM.y, - 10]) 
      linear_extrude(height=10, center=!true, convexity=10, twist=0)
      square(size=[EEE_BODY.x, EEE_LID_BOTTOM.y*2], center=true);    
    }
    
    // window
    linear_extrude(height=10, center=true, convexity=10, twist=0)
    translate_to_lens_effective_center()
    rounding(r=1.5)
    square(size=[LENS_EFFECTIVE.x, LENS_EFFECTIVE.y], center=true);

    // lid space
*    translate([0, 0, EEE_BODY.z - EEE_LID.z]) 
    linear_extrude(height=10, center=!true, convexity=10, twist=0)
    offset(delta=0.5)
    rounding(r=3)
    square(size=[EEE_LID.x, EEE_LID.y], center=true);
  }
}

module translate_to_lcd_z_plane() {
   translate([0, 0, EEE_BODY.z - EEE_LID.z - LCD_MODULE_BODY.z - EEE_LCD_MOUNT_Z]) 
   children(0);
}

module translate_to_lid_back_z_plane() {
   translate([0, 0, EEE_BODY.z - EEE_LID.z]) 
   children(0);
}

module eee_legs_body() {
  mirror_x()
  translate([EEE_BODY.x/2 - 20, -EEE_BODY.y/2, 5]) 
  hull(){
    // edge
    translate([0, 0, EEE_LEG.z]) 
    rotate([0, 90, 0])
    cylinder(r=0.1, h=EEE_LEG.x, center=true);

    // main
    linear_extrude(height=EEE_LEG.z/2, center=!true, convexity=10, twist=0)
    difference() { // cut it half
      rounding(r=2)
      square(size=[EEE_LEG.x, EEE_LEG.y*2], center=true);
    
      translate([0, EEE_LEG.y/2, 0]) 
      square(size=[EEE_LEG.x*2, EEE_LEG.y], center=true);
    }
  }
}

module translate_to_lid_back_holes() {
  translate([EEE_LID.x/2-2.5, EEE_LID.y/2-5, 0]) 
  children(0);
}

module eee_lid_back_holes(d=2) {
  mirror_x()
  translate_to_lid_back_holes() 
  circle(d=d);
}

module translate_to_lid_bottom_holes() {
   translate([0, -EEE_BODY.y/2 + EEE_LID_BOTTOM.y, 10]) 
   children(0);
}

module eee_lid_bottom_holes(d=2) {
  mirror_x()
  translate([EEE_LID.x/2, 0, 0]) 
  circle(d=d);
}

module eee_lid_base() {
  intersection(){
    eee_body_base();

    union(){
      // back
      translate([0, 0, EEE_BODY.z - EEE_LID.z]) 
      linear_extrude(height=10, center=!true, convexity=10, twist=0)
      offset(delta=0.5)
      rounding(r=3)
      square(size=[EEE_LID.x, EEE_LID.y], center=true);

      // bottom
      translate([0, -EEE_BODY.y/2, EEE_BODY.z/2 - 0.01]) 
      cube(size=[EEE_LID_BOTTOM.x+2, EEE_LID_BOTTOM.y*2, EEE_LID_BOTTOM.z+2], center=true);
    }
  }
  
  // legs (Type L has legs attached to lid)
  eee_legs_body();
}

module eee_lid_cover_mount_holes(d=2) {
  mirror_x()
  translate([EEE_LID.x/2 - 4, 0, 0]) 
  circle(d=d);
}

module eee_lid_body() {
  difference(){
    eee_lid_base();

    // lid back screw holes 
    translate_to_lid_back_z_plane()
    linear_extrude(height=10, center=true, convexity=10, twist=0)
    eee_lid_back_holes(d=2.5);

    // lid bottom scres holes
    translate_to_lid_bottom_holes()
    translate([0, -2, 0]) 
    rotate([90, 0, 0]){
      // space for screw head
      linear_extrude(height=EEE_LID_BOTTOM.y*2, center=!true, convexity=10, twist=0)
      eee_lid_bottom_holes(d=3.5);

      // space for screw
      linear_extrude(height=EEE_LID_BOTTOM.y*2, center=true, convexity=10, twist=0)
      eee_lid_bottom_holes(d=2);
    }
  }

  // LCD mount
  translate([0, 0, EEE_BODY.z - EEE_LID.z - EEE_LCD_MOUNT_Z])
  linear_extrude(height=EEE_LCD_MOUNT_Z, center=!true, convexity=10, twist=0)
  translate_to_lcd_center(as_center=true)
  translate_to_lens_effective_center()
  difference() {
    lcd_module_holes(d=4);
    lcd_module_holes(d=1.8);
  }

  // LCD cover mount
  translate([0, 0, EEE_BODY.z - EEE_LID.z - EEE_LCD_COVER_MOUNT_Z])
  linear_extrude(height=EEE_LCD_COVER_MOUNT_Z, center=!true, convexity=10, twist=0)
  translate_to_lens_effective_center()
  difference() {
    eee_lid_cover_mount_holes(d=5);
    eee_lid_cover_mount_holes(d=1.8);
  }
}

module lid_back_mount_base(args) {
  mirror_x()
  translate_to_lid_back_z_plane()
  translate_to_lid_back_holes()
  rotate([0, 180, 0])
  hull(){
    linear_extrude(height=2, center=!true, convexity=10, twist=0)
    square(size=[EEE_LID_MOUNT_BACK.x, EEE_LID_MOUNT_BACK.y], center=true);

    translate([-EEE_LID_MOUNT_BACK.x/2, 0, EEE_LID_MOUNT_BACK.z]) 
    rotate([90, 0, 0])
    cylinder(d=0.1, h=EEE_LID_MOUNT_BACK.y, center=true);
  }
}

module lid_bottom_mount_base(along_y=false) {
  if(along_y){
    mirror_x()
    translate([-EEE_LID.x/2 + EEE_LID_MOUNT_BOTTOM.x/2, 0, 0]) 
    translate_to_lid_bottom_holes()
    rotate([-90, 0, 0])
    hull(){
      linear_extrude(height=2, center=!true, convexity=10, twist=0)
      square(size=[EEE_LID_MOUNT_BOTTOM.x, EEE_LID_MOUNT_BOTTOM.y], center=true);

      translate([-EEE_LID_MOUNT_BOTTOM.x/2, 0, EEE_LID_MOUNT_BOTTOM.z]) 
      rotate([90, 0, 0])
      cylinder(d=0.1, h=EEE_LID_MOUNT_BOTTOM.y, center=true);
    }
  }else{
    mirror_x()
    translate([-EEE_LID.x/2 + EEE_LID_MOUNT_BOTTOM.x/2, EEE_LID_MOUNT_BOTTOM.y/2, EEE_LID_MOUNT_BOTTOM.z/2-1]) 
    translate_to_lid_bottom_holes()
    rotate([-180, 0, 0])
    hull(){
      linear_extrude(height=2, center=!true, convexity=10, twist=0)
      square(size=[EEE_LID_MOUNT_BOTTOM.x, EEE_LID_MOUNT_BOTTOM.y], center=true);

      translate([-EEE_LID_MOUNT_BOTTOM.x/2, 0, EEE_LID_MOUNT_BOTTOM.z]) 
      rotate([90, 0, 0])
      cylinder(d=0.1, h=EEE_LID_MOUNT_BOTTOM.y, center=true);
    }
  }
}

module eee_body() {
  difference() {
    union(){
      eee_body_base();

      // lid back mount
      lid_back_mount_base();

      // lid bottom mount
      lid_bottom_mount_base();
    }

    eee_lid_base();

    // lid back mount holes
    translate_to_lid_back_z_plane()
    linear_extrude(height=10, center=true, convexity=10, twist=0)
    eee_lid_back_holes(d=1.8);

    // lid bottom mount holes
    translate_to_lid_bottom_holes()
    rotate([90, 0, 0]){
      // space for screw
      linear_extrude(height=EEE_LID_BOTTOM.y*2, center=true, convexity=10, twist=0)
      eee_lid_bottom_holes(d=1.8);
    }

    // cable hole
    translate([EEE_BODY.x/2, -15, EEE_BODY.z]) 
    rotate([0, 90, 0])
    hull() {
      cylinder(d=3, h=10, center=true);

      translate([2, 0, 0]) 
      cylinder(d=3, h=10, center=true);
    }
  }
}
