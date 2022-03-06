include <modules/eee_case.scad>

%
lens_body();

eee_body();

%
translate_to_lens_effective_center()
translate_to_lcd_z_plane()
translate_to_lcd_center(as_center=true)
lcd_module_body();
