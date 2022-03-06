include <../scad-utils/morphology.scad>

LENS_BODY=[85.5,54.3,0.4];
LENS_EFFECTIVE=[80.0, 43.0, LENS_BODY.z];
LENS_TO_DISPLAY=30;


module translate_to_lens_effective_center(){
  translate([0, (LENS_BODY.y - LENS_EFFECTIVE.y)/2 - 3, 0]) 
  children(0);
}

module lens_body() {
  rounding(r=2)
  square(size=[LENS_BODY.x, LENS_BODY.y], center=true);

  color("yellow", 0.2)
  translate_to_lens_effective_center()
  rounding(r=1.5)
  square(size=[LENS_EFFECTIVE.x, LENS_EFFECTIVE.y], center=true);
}
