include <../scad-utils/morphology.scad>
include <./flesnel_lens.scad>
include <./lcd_096in_st7735.scad>

$fn=50;

// EEE_BODY=[LENS_BODY.x + 2.5, LENS_BODY.y + 2.5, 30];
EEE_LCD_MOUNT_Z=2.5;
EEE_BODY=[LENS_BODY.x, LENS_BODY.y, LENS_TO_DISPLAY + LCD_MODULE_TOTAL_Z + EEE_LCD_MOUNT_Z];
EEE_BODY_FRONT_Z=0.6;

FUNNEL_BODY=[LENS_EFFECTIVE.x, LENS_EFFECTIVE.y, LENS_TO_DISPLAY];

EEE_LEG=[10, 3, 6];

EEE_LID=[EEE_BODY.x - 3, EEE_BODY.y - 3, 1];

module eee_body_base() {
  difference() {
    linear_extrude(height=EEE_BODY.z, center=!true, convexity=10, twist=0)
    rounding(r=3)
    square(size=[EEE_BODY.x, EEE_BODY.y], center=true);

    // inner space
    translate([0, 0, EEE_BODY_FRONT_Z]) 
    linear_extrude(height=EEE_BODY.z, center=!true, convexity=10, twist=0)
    rounding(r=3)
    inset(d=2)
    square(size=[EEE_BODY.x, EEE_BODY.y], center=true);
    
    // window
*    linear_extrude(height=10, center=true, convexity=10, twist=0)
    translate_to_lens_effective_center()
    rounding(r=1.5)
    square(size=[LENS_EFFECTIVE.x, LENS_EFFECTIVE.y], center=true);

    // lid space
    translate([0, 0, EEE_BODY.z - EEE_LID.z]) 
    linear_extrude(height=10, center=!true, convexity=10, twist=0)
    offset(delta=0.5)
    rounding(r=3)
    square(size=[EEE_LID.x, EEE_LID.y], center=true);
  }
}

module funnel_body_base(offset=0, z_delta=0) {
  hull(){
    translate([0, 0, FUNNEL_BODY.z + z_delta])
    linear_extrude(height=0.1, center=!true, convexity=10, twist=0)
    offset(delta=offset)
    offset(r=1)
    rounding(r=1)
    square(size=[LCD_MODULE_LCD.x, LCD_MODULE_LCD.y], center=true);

    translate([0, 0, -z_delta]) 
    linear_extrude(height=0.1, center=!true, convexity=10, twist=0)
    offset(delta=offset)
    offset(r=1)
    rounding(r=1.5)
    square(size=[LENS_EFFECTIVE.x, LENS_EFFECTIVE.y], center=true);
  }
}

module translate_to_lcd_z_plane() {
   translate([0, 0, EEE_BODY_FRONT_Z + FUNNEL_BODY.z + LCD_MODULE_LCD.z]) 
   children(0);
}

module funnel_body() {
  difference(){
    funnel_body_base();

    // inner space  
    funnel_body_base(offset=-1, z_delta=0.01);
  }
}

module eee_legs_body() {
  mirror_x()
  translate([EEE_BODY.x/2 - 20, -EEE_BODY.y/2, 5]) 
  hull(){
    // edge
    rotate([0, 90, 0])
    cylinder(r=0.1, h=EEE_LEG.x, center=true);

    translate([0, 0, EEE_LEG.z/2]) 
    linear_extrude(height=EEE_LEG.z/2, center=!true, convexity=10, twist=0)
    difference() {
      rounding(r=2)
      square(size=[EEE_LEG.x, EEE_LEG.y*2], center=true);
    
      translate([0, EEE_LEG.y/2, 0]) 
      square(size=[EEE_LEG.x*2, EEE_LEG.y], center=true);
    }
  }
}

module eee_lid_holes(d=2) {
  mirror_x()
  mirror_y()
  translate([EEE_LID.x/2-2.5, EEE_LID.y/2-2.5, 0]) 
  circle(d=d);
}

module eee_lid_body() {
  difference(){
    linear_extrude(height=EEE_LID.z, center=!true, convexity=10, twist=0)
    difference(){
      rounding(r=3)
      square(size=[EEE_LID.x, EEE_LID.y], center=true);

      // holes
      eee_lid_holes();
    }

    // screw head
    translate([0, 0, EEE_LID.z - 0.3]) 
    linear_extrude(height=0.3, center=!true, convexity=10, twist=0)
    eee_lid_holes(3);
  }

  // LCD mount
  translate([0, 0, -EEE_LCD_MOUNT_Z + EEE_LID.z])
  linear_extrude(height=EEE_LCD_MOUNT_Z, center=!true, convexity=10, twist=0)
  translate_to_lcd_center(as_center=true)
  translate_to_lens_effective_center()
  difference() {
    lcd_module_holes(d=4);
    lcd_module_holes(d=1.8);
  }
}

module eee_body() {
  difference() {
    union(){
      eee_body_base();

      // lid mount
      linear_extrude(height=EEE_BODY.z - EEE_LID.z, center=!true, convexity=10, twist=0)
      eee_lid_holes(4);

      // funnel
      translate_to_lens_effective_center()
      funnel_body_base();

      // leg
      eee_legs_body();
    }

    // lid mount holes
    translate([0, 0, EEE_BODY.z - 5]) 
    linear_extrude(height=5, center=!true, convexity=10, twist=0)
    eee_lid_holes(1.8);

    // funnel inner space & window
    translate_to_lens_effective_center()
    funnel_body_base(offset=-1, z_delta=0.01);

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
