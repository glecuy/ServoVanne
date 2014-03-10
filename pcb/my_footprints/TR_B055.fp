#Element [element_flags, description, pcb-name, value, mark_x, mark_y, text_x, text_y, text_direction, text_scale, text_flags]
#Pad [x1 y1 x2 y2 thickness clearance mask name pad_number flags]
#Pin [x y thickness clearance mask drillholedia name number flags]
#ElementArc [x y r1 r2 startangle sweepangle thickness]
#ElementLine [x1 y1 x2 y2 thickness] –> thickness != 1000 = 10 mils almost for all footprints
# See 
# http://wiki.geda-project.org/geda:pcb-quick_reference
# http://www.brorson.com/gEDA/land_patterns_20070805.pdf
# http://members.impulse.net/~uhl/utilities/geda_fp_creator/fp_creator.html

Element["" "TR_B055" "TR?" "TR_B055" 0 0 0 -86613 0 100 ""]
(
Pin[-49212 -49212 11811 2500 2500 2755  "1"   "1" 0x00000000]
Pin[ 49212 -49212 11811 2500 2500 2755 "10"  "10" 0x00000000]
Pin[ 49212 -29527 11811 2500 2500 2755  "9"   "9" 0x00000000]
Pin[-49212  49212 11811 2500 2500 2755  "5"   "5" 0x00000000]
Pin[ 49212  29527 11811 2500 2500 2755  "7"   "7" 0x00000000]
Pin[ 49212  49212 11811 2500 2500 2755  "6"   "6" 0x00000000]
ElementLine[-72834 -86613 -72834 86613 1000]
ElementLine[-72834 86613 72834 86613 1000]
ElementLine[72834 86613 72834 -86613 1000]
ElementLine[72834 -86613 -72834 -86613 1000]

ElementLine[-55000 -65000 -55000 65000 1000]
ElementLine[-55000 65000 55000 65000 1000]
ElementLine[55000 65000 55000 -65000 1000]
ElementLine[55000 -65000 -55000 -65000 1000]
)

