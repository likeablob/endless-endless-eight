include <../scad-utils/morphology.scad>

LCD_MODULE_BODY=[30, 24, 1.2];
LCD_MODULE_LCD=[24.7, 13.5, 1.5];
LCD_MODULE_TOTAL_Z=LCD_MODULE_BODY.z + LCD_MODULE_LCD.z;


module lcd_module_holes(d=2.5) {
  mirror_x()
  mirror_y()
  translate([LCD_MODULE_BODY.x/2-2.5, LCD_MODULE_BODY.y/2-2.5, 0]) 
  circle(d=d);
}

module translate_to_lcd_center(as_center=false){
  translate([((LCD_MODULE_BODY.x - LCD_MODULE_LCD.x)/2 - 0.3) * (as_center ? 1 : -1), 0, 0]) 
  children(0);
}

module lcd_module_body(){
  linear_extrude(height=LCD_MODULE_BODY.z, center=!true, convexity=10, twist=0)
  difference() {
    rounding(r=1)
    square(size=[LCD_MODULE_BODY.x, LCD_MODULE_BODY.y], center=true);
    
    lcd_module_holes();
  }

  // LCD
  color("lightgreen", 0.5)
  translate_to_lcd_center()
  translate([0, 0, -LCD_MODULE_LCD.z/2]) 
  cube(size=LCD_MODULE_LCD, center=true);
}
