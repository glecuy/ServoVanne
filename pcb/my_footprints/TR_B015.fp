#Element [element_flags, description, pcb-name, value, mark_x, mark_y, text_x, text_y, text_direction, text_scale, text_flags]
#Pad [x1 y1 x2 y2 thickness clearance mask name pad_number flags]
#Pin [x y thickness clearance mask drillholedia name number flags]
#ElementArc [x y r1 r2 startangle sweepangle thickness]
#ElementLine [x1 y1 x2 y2 thickness] –> thickness != 1000 = 10 mils almost for all footprints
# See 
# http://wiki.geda-project.org/geda:pcb-quick_reference
# http://www.brorson.com/gEDA/land_patterns_20070805.pdf
# http://members.impulse.net/~uhl/utilities/geda_fp_creator/fp_creator.html

Element["" "Hello" "TR?" "Hello" 0 0 0 -63622 0 100 ""]
(
Pin[-40000 -40000 8000 2500 2500 2000  "1"   "1" 0x00000000]
Pin[-40000  40000 8000 2500 2500 2000  "5"   "5" 0x00000000]
Pin[ 40000 -40000 8000 2500 2500 2000 "10"  "10" 0x00000000]
Pin[ 40000 -20000 8000 2500 2500 2000  "9"   "9" 0x00000000]
Pin[ 40000      0 8000 2500 2500 2000  "8"   "8" 0x00000000]
ElementLine[-51811 -63622 -51811 63622 1000]
ElementLine[-51811 63622 51811 63622 1000]
ElementLine[51811 63622 51811 -63622 1000]
ElementLine[51811 -63622 -51811 -63622 1000]
ElementLine[-35433 -43307 -35433 43307 1000]
ElementLine[-35433 43307 35433 43307 1000]
ElementLine[35433 43307 35433 -43307 1000]
ElementLine[35433 -43307 -35433 -43307 1000]
)

