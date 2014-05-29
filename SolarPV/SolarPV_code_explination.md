<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<!-- saved from url=(0230)https://mail-attachment.googleusercontent.com/attachment/u/0/?ui=2&ik=74648b41b9&view=att&th=14642faf760e84f6&attid=0.1&disp=inline&safe=1&zw&saduie=AG9B_P8suJ31eAWuBhSeFxvwn4os&sadet=1401352163405&sads=Gz1_Vaiso8tfZyEs0ZrfynXxo4Y -->
<html><head><meta http-equiv="Content-Type" content="text/html; charset=UTF-8"><style id="clearly_highlighting_css" type="text/css">/* selection */ html.clearly_highlighting_enabled ::-moz-selection { background: rgba(246, 238, 150, 0.99); } html.clearly_highlighting_enabled ::selection { background: rgba(246, 238, 150, 0.99); } /* cursor */ html.clearly_highlighting_enabled {    /* cursor and hot-spot position -- requires a default cursor, after the URL one */    cursor: url("chrome-extension://pioclpoplcdbaefihamjohnefbikjilc/clearly/images/highlight--cursor.png") 14 16, text; } /* highlight tag */ em.clearly_highlight_element {    font-style: inherit !important; font-weight: inherit !important;    background-image: url("chrome-extension://pioclpoplcdbaefihamjohnefbikjilc/clearly/images/highlight--yellow.png");    background-repeat: repeat-x; background-position: top left; background-size: 100% 100%; } /* the delete-buttons are positioned relative to this */ em.clearly_highlight_element.clearly_highlight_first { position: relative; } /* delete buttons */ em.clearly_highlight_element a.clearly_highlight_delete_element {    display: none; cursor: pointer;    padding: 0; margin: 0; line-height: 0;    position: absolute; width: 34px; height: 34px; left: -17px; top: -17px;    background-image: url("chrome-extension://pioclpoplcdbaefihamjohnefbikjilc/clearly/images/highlight--delete-sprite.png"); background-repeat: no-repeat; background-position: 0px 0px; } em.clearly_highlight_element a.clearly_highlight_delete_element:hover { background-position: -34px 0px; } /* retina */ @media (min--moz-device-pixel-ratio: 2), (-webkit-min-device-pixel-ratio: 2), (min-device-pixel-ratio: 2) {    em.clearly_highlight_element { background-image: url("chrome-extension://pioclpoplcdbaefihamjohnefbikjilc/clearly/images/highlight--yellow@2x.png"); }    em.clearly_highlight_element a.clearly_highlight_delete_element { background-image: url("chrome-extension://pioclpoplcdbaefihamjohnefbikjilc/clearly/images/highlight--delete-sprite@2x.png"); background-size: 68px 34px; } } </style><style type="text/css"></style><style>[touch-action="none"]{ -ms-touch-action: none; touch-action: none; }[touch-action="pan-x"]{ -ms-touch-action: pan-x; touch-action: pan-x; }[touch-action="pan-y"]{ -ms-touch-action: pan-y; touch-action: pan-y; }[touch-action="scroll"],[touch-action="pan-x pan-y"],[touch-action="pan-y pan-x"]{ -ms-touch-action: pan-x pan-y; touch-action: pan-x pan-y; }</style><script>window["_GOOG_TRANS_EXT_VER"] = "1";</script></head><body><h3>SolarPV.ino — how it works.</h3>
<p>By Robert Wall - May 2014</p>

<p>The sketch depends on two other files, “<i>icons.ino</i>” and “<i>templates.ino</i>”
as well as many libraries. All the requirements are listed in a comment at the beginning of the sketch.</p>

<p>Following this are definitions for setting the RFM12B radio module, the structures for transferring the data, 
some constants for the GLCD itself (including arrays for storing the energy use history), definitions for the
temperature sensor and finally the page control.</p>

<p><b>setup( )</b></p>
<p>In the setup( ) function, the RFM12B radio is initialised, followed by the GLCD, the backlight brightness is set, 
the temperature sensor is initialised, and the input and output pins set appropriately.</p>

<p>After the setup has completed, the operating system repeatedly invokes the “loop( )” function, it is 
inside there that the ongoing processing of the data takes place. 
</p>

<p><b>loop( )</b></p>
<p>First, the RFM12B is checked to see if a message has been received. If it has, the Node ID is extracted and 
compared to the known fixed ID's for the emonTx and the emonBase. If the message is from the emonTx, the message is
copied into the structure “emontx”. If it is from the Base, the byte values are applied directly to
update the real-time clock.</p>
<p>After that, there are just two “<i>if …</i>”  statements. The first block is the fast update which 
updates the display every 200 ms, the second is the slow update which happens every 10 s.</p>

<p><b>The fast 'if' block</b></p>
<p>The time is retrieved from the clock, ready to be written to the display. Then, the present power values power1 
and power2 are converted into energy (assuming it was constant for the last 200 ms) and accumulated in the 
“<i>…kwh</i>” variables, account being taken of the meaning of the two numbers (a 'Type 1' or a 'Type 2' installation).</p>
<p>Next, midnight is detected (“if (last_hour == 23 …”) and the daily accumulated values are moved one 
place down the array and the new day’s value set to zero.</p>
<p>The running power values (“cval_use = …”) are then low-pass filtered to alleviate flicker and 
true values for power used and power generated and found.</p>
<p>Next, the push-button switches are interrogated (“last_switch_state = …”) and the page changed on a 
switch press. 
</p>
<p>Following this are two more “if …”  blocks for the two pages of the display. These simply pass 
parameters to the function that actually draws the page on the display. 
</p>

<p>The remainder of the fast block deals with driving the coloured LEDs and controlling the brightness of the 
backlight. The comments explain this.</p>

<p><b>The slow 'if' block</b></p>
<p>This reads the temperature from the on-board sensor, checks that the value is sensible and sends it by radio 
to the Base for logging. The maximum and minimum values are recorded for display.</p>

<h3>templates.ino — How it works.</h3>
<p>This file contains the functions that write to the display itself.</p>
<p>Rather than describing the process line by line, it should suffice to explain in general how data and text 
is written to the display.</p>
<p>The examples are from the function “draw_solar_page( )”.</p>
<p>The display unit is the pixel. The starting position is top left. Characters are just a collection of pixels.
When writing to the display, characters can overlap and jumble, therefore when updating the display it is 
necessary to erase the area first. A call to “glcd.clear” and “glcd.fillRect(0,0,128,64,0)” 
will erase the previous data from the buffer memory and set the rectangle at pixel column 0, row 0 to column 128, 
row 64 (all of the display) to colour 0, effectively giving a blank display.</p>
<p>Anything written to the display remains hidden until “glcd.refresh()” is called, when the 
entire buffer memory is written to the screen.</p> 
<p>Lines can be drawn: “glcd.drawLine(64, 0, 64, 64, WHITE)” which draws from pixel column 64, row 0
to pixel column 64, row 64. </p>
<p>Text can be written in one of a selection of fonts, the files defining these were included at the top of the file.
The font is set with “glcd.setFont(font_clR4x6)” and it remains in force until superseded.</p>
<p>There are two choices to put something on the display: text or a bitmap image. Text (which includes numbers) is
written with “glcd.drawString_P(40,0,PSTR("<wbr>History"))”. This writes the word “History” starting with the top left corner at pixel column 40, row 0.  If a number needs to be written, it must
first be converted to a string and temporarily stored: “itoa(emonTx_fail, str, 10)” takes an integer 
in emonTx_fail and stores it in str with a base 10 (decimal) conversion. There are similar conversion functions for
the other number types, e.g. “dtostrf(use/1000,2,1,str)” which outputs a decimal number with a 
minimum width of 2 characters with 1 decimal digit.</p>
<p>To draw an image, (which is defined in the icons.ino file) a call to 
“glcd.drawBitmap(51,34,icon_<wbr>lines_12x12,16,12,1)”
is made. This locates the top left corner at pixel column 51, line 34 and puts there the bitmap 
“icon_lines_12x12” with the width 16 px and height 12 px, colour 1.</p>
<p>A list of the library class members is at <a href="http://jeelabs.net/pub/docs/glcdlib/classGLCD__ST7565-members.html" target="_blank">http://jeelabs.net/pub/docs/<wbr>glcdlib/classGLCD__ST7565-<wbr>members.html</a>

</p></body></html>