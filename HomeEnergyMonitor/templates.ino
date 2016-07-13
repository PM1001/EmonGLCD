
int qlog10(int num) {
  if (num >= 1000000)
    return 6;
  if (num >= 100000)
    return 5;
  if (num >= 10000)
    return 4;
  if (num >= 1000)
    return 3;
  if (num >= 100)
    return 2;
  if (num >= 10)
    return 1;
  return 0;
}
void  printTime(byte x, byte y,byte hour, byte minute,byte width=6){
    char chr[2]="0";
    chr[0] = '0'+(hour/10);
    glcd.drawStr(x+5, y,chr);
    chr[0] = '0'+(hour%10);
    glcd.drawStr(x+width+5, y, chr);
    glcd.drawStr(x+width*2+4, y-2, ":");
    chr[0] = '0'+(minute/10);
    glcd.drawStr(x+width*3-1, y, chr);
    chr[0] = '0'+(minute%10);
    glcd.drawStr(x+width*4-1, y, chr);
  
}
//------------------------------------------------------------------
// Draws a page showing a single power and energy value in big font
//------------------------------------------------------------------
void draw_power_page(double powerval,  double energyval,double temp, double mintemp, double maxtemp, byte hour, byte minute,int16_t geyserTemp,int16_t outsideTemp,int16_t voltage,int16_t frequency,int16_t geyserManifold,int16_t pumpRegister)
{ 
  glcd.setFont(u8g_font_fub17n);
  glcd.setFontPosTop();  
  
  glcd.setPrintPos(4*12-12* qlog10(powerval), 0);
  glcd.print(powerval, 0); // 0 dp's
  printTime(128-60,0,hour,minute,12);
  
  glcd.setPrintPos(2*12-8-12* qlog10(energyval/100.0), 18);
  glcd.print(energyval/100.0, 2); // 0 dp's
  
    // Write small text to LCD (col, row, text) - Ch1
  glcd.setFont(u8g_font_6x10);
  glcd.setFontPosTop();  
  glcd.drawStr(64, 0, "W");
  
  glcd.drawStr(64, 18, "kWh");

  glcd.setPrintPos(3*6-6* qlog10(voltage/10), 36);
  glcd.print(voltage/10.0, 1); // 0 dp's
  glcd.drawStr(36, 36, "V");
  
  glcd.setPrintPos(42+2*6-6* qlog10(frequency/10), 36);
  glcd.print(frequency/10.0, 1); // 0 dp's
  glcd.drawStr(42+30, 36, "Hz");
  
  glcd.drawLine(0,63-17,83,63-17);
  glcd.drawStr(0, 63-16, "Inside");
  glcd.setPrintPos(0, 63-8);
  glcd.print(temp, 1); // 0 dp's
  char chr[3]=" C";
  chr[0] = char(176);
  glcd.drawStr(6*4, 63-8, chr);
  
  
  glcd.drawStr(42, 63-16, "Outsde");
  glcd.setPrintPos(42, 63-8);
  glcd.print(outsideTemp/10.0, 1); // 0 dp's
  glcd.drawStr(42+6*4, 63-8, chr);
  
//  glcd.drawStr(128-36, 63-16, "Geyser");


  glcd.drawLine(82,19,99,36);
  glcd.drawLine(85,22,85,62);
  glcd.drawLine(88,25,88,59);
  glcd.drawLine(89,59,104,59);
  glcd.drawLine(85,62,104,62);
  glcd.drawLine(90,62,104,62);
  glcd.drawFrame(104, 38, 24, 26);
  if(RTC.now().second()%2){
    if((pumpRegister/10)&8)
      glcd.drawFrame(109, 58, 14, 6);
    if((pumpRegister/10)&49){     
      glcd.drawStr(90,50, "P");
    }
    
//        glcd.setPrintPos(90,51);
//        glcd.print(pumpRegister/10.0, 1); // 0 dp's
    
  }
  glcd.setPrintPos(110, 42);
  glcd.print(geyserTemp/10, 1); // 0 dp's
  glcd.setPrintPos(99, 22);
  glcd.print(geyserManifold/10, 1); // 0 dp's
}



