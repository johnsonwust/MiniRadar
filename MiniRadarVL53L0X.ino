//淘宝『创元素店』https://shop423015102.taobao.com/
//更新日期 2021/03/06
//MiniRadar 超声波雷达 程序
//本程序对应商品 https://item.taobao.com/item.htm?spm=a1z10.1-c.w4004-23815833841.8.4f231fe7qvLFZi&id=649834806872

//Github版链接：  https://github.com/johnsonwust/MiniRadar

#include <Servo.h> 
#include <SPI.h>
#include "Ucglib.h"           
//显示屏的lib 如果没有该lib请按Ctrl+Shift+I 从 库管理器中搜索 ucglib，并安装
#include <Wire.h>

#define VL53L0X_REG_SYSRANGE_START                  0x00
#define VL53L0X_REG_RESULT_RANGE_STATUS             0x14
#define VL53L0X_I2C_ADDRESS                         0x29

class VL53L0X {
  private:
    byte gbuf[16];  // 私有缓冲区

    uint16_t makeuint16(int lsb, int msb) {
      return ((msb & 0xFF) << 8) | (lsb & 0xFF);
    }

    void write_byte_data_at(byte reg, byte data) {
      Wire.beginTransmission(VL53L0X_I2C_ADDRESS);
      Wire.write(reg);
      Wire.write(data);
      Wire.endTransmission();
    }

    byte read_byte_data_at(byte reg) {
      Wire.beginTransmission(VL53L0X_I2C_ADDRESS);
      Wire.write(reg);
      Wire.endTransmission();
      Wire.requestFrom(VL53L0X_I2C_ADDRESS, 1);
      while (Wire.available() < 1) delay(1);
      return Wire.read();
    }

    void read_block_data_at(byte reg, int sz) {
      Wire.beginTransmission(VL53L0X_I2C_ADDRESS);
      Wire.write(reg);
      Wire.endTransmission();
      Wire.requestFrom(VL53L0X_I2C_ADDRESS, sz);
      for (int i=0; i<sz; i++) {
        while (Wire.available() < 1) delay(1);
        gbuf[i] = Wire.read();
      }
    }

  public:
    void begin() {
      Wire.begin();
    }

    void startMeasurement() {
      write_byte_data_at(VL53L0X_REG_SYSRANGE_START, 0x01);
    }

    bool isReady() {
      byte val = 0;
      int cnt = 0;
      while (cnt < 100) { // 最大等待时间1秒
        delay(10);
        val = read_byte_data_at(VL53L0X_REG_RESULT_RANGE_STATUS);
        if (val & 0x01) break;
        cnt++;
      }
      return (val & 0x01);
    }

    uint16_t getDistance() {
      read_block_data_at(0x14, 12);
      return makeuint16(gbuf[11], gbuf[10]);
    }
};

VL53L0X sensor;

#define  ServoPin  3          //底座舵机端口 3#
int Ymax = 128;               //屏幕的竖向像素数
int Xmax = 160;               //屏幕的横向像素数
int Xcent = Xmax / 2;         //x中位
int base = 118;               //基线高度
int scanline = 105;           //雷达扫描线长度

Servo baseServo; 
Ucglib_ST7735_18x128x160_HWSPI ucg(/*cd=*/ 9, /*cs=*/ 10, /*reset=*/ 8);

void setup(void)
{
      sensor.begin();
 
      ucg.begin(UCG_FONT_MODE_SOLID); //初始化屏幕
      ucg.setRotate90();              //设置成横屏  如果屏幕显示方向是反的，可以修改函数 setRotate90 或 setRotate270

      Serial.begin(115200);             //设置串口传输率
      baseServo.attach(ServoPin);     //初始化舵机
    
      //欢迎屏幕
      ucg.setFontMode(UCG_FONT_MODE_TRANSPARENT);
      ucg.setColor(0, 0, 100, 0);
      ucg.setColor(1, 0, 100, 0);
      ucg.setColor(2, 20, 20,20);
      ucg.setColor(3, 20, 20, 20);
      ucg.drawGradientBox(0, 0, 160, 128);
      ucg.setPrintDir(0);
      ucg.setColor(0, 5, 0);
      ucg.setPrintPos(27,42);
      ucg.setFont(ucg_font_logisoso18_tf);  
      ucg.print("Mini Radar");
      ucg.setColor(0, 255, 0);
      ucg.setPrintPos(25,40);
      ucg.print("Mini Radar");
      ucg.setFont(ucg_font_helvB08_tf);
      ucg.setColor(20, 255, 20);
      ucg.setPrintPos(40,100);
      ucg.print("Testing...");
      baseServo.write(90);
    
      //测试底座的运行情况，注意检测底座位置和转动姿态，是否有卡住（或者导线缠绕）的情况。
      for(int x=0;x<180;x+=5)
          { baseServo.write(x);
            delay(50);
           }
      ucg.print("OK!");
      delay(500);

    
      //清屏
      //ucg.clearScreen();
      cls();
      ucg.setFontMode(UCG_FONT_MODE_SOLID);
      ucg.setFont(ucg_font_orgv01_hr);
  
}


void cls()
{
  //清屏
  ucg.setColor(0, 0, 0, 0);
  
  for(int s=0;s<128;s+=8)
  for(int t=0;t<160;t+=16)
  {
    ucg.drawBox(t,s,16,8);
   // delay(1);
  }
    

}

void fix_font() 
{
      ucg.setColor(0, 180, 0);
      ucg.setPrintPos(70,14);
      ucg.print("2.00");
      ucg.setPrintPos(70,52);
      ucg.print("1.00");
      ucg.setPrintPos(70,90);
      ucg.print("0.50");
}

void fix()
{

      ucg.setColor(0, 40, 0);
      //画基线圆盘
      ucg.drawDisc(Xcent, base+1, 3, UCG_DRAW_ALL); 
      ucg.drawCircle(Xcent, base+1, 115, UCG_DRAW_UPPER_LEFT);
      ucg.drawCircle(Xcent, base+1, 115, UCG_DRAW_UPPER_RIGHT);
      ucg.drawCircle(Xcent, base+1, 78, UCG_DRAW_UPPER_LEFT);
      ucg.drawCircle(Xcent, base+1, 78, UCG_DRAW_UPPER_RIGHT);
      ucg.drawCircle(Xcent, base+1, 40, UCG_DRAW_UPPER_LEFT);
      ucg.drawCircle(Xcent, base+1, 40, UCG_DRAW_UPPER_RIGHT);
      ucg.drawLine(0, base+1, Xmax,base+1);
     
      ucg.setColor(0, 120, 0);
      //画刻度表
       for(int i= 40;i < 140; i+=2)
       {

        if (i % 10 == 0) 
          ucg.drawLine(105*cos(radians(i))+Xcent,base - 105*sin(radians(i)) , 113*cos(radians(i))+Xcent,base - 113*sin(radians(i)));
        else
        
         ucg.drawLine(110*cos(radians(i))+Xcent,base - 110*sin(radians(i)) , 113*cos(radians(i))+Xcent,base - 113*sin(radians(i)));
       }
          
       //画一些装饰性图案 
       ucg.setColor(0,200,0);
       ucg.drawLine(0,0,0,18);
       for(int i= 0;i < 5; i++)
       {
          ucg.setColor(0,random(200)+50,0);
          ucg.drawBox(2,i*4,random(14)+2,3);
       }

       ucg.setColor(0,180,0);
       ucg.drawFrame(146,0,14,14);
       ucg.setColor(0,60,0);
       ucg.drawHLine(148,0,10);
       ucg.drawVLine(146,2,10);
       ucg.drawHLine(148,13,10);
       ucg.drawVLine(159,2,10);
        
       ucg.setColor(0,220,0);
       ucg.drawBox(148,2,4,4);
       ucg.drawBox(148,8,4,4);
       ucg.drawBox(154,8,4,4);
       ucg.setColor(0,100,0);
       ucg.drawBox(154,2,4,4);

       ucg.setColor(0,90,0);
       ucg.drawTetragon(62,123,58,127,98,127,102,123);
       ucg.setColor(0,160,0);
       ucg.drawTetragon(67,123,63,127,93,127,97,123);
       ucg.setColor(0,210,0);
       ucg.drawTetragon(72,123,68,127,88,127,92,123);
}



void loop(void)
{
  
  int distance;
  
  fix(); 
  fix_font();  //重绘屏幕背景元素

  for (int x=180; x > 4; x-=2){       //底座舵机从180~0度循环
     
      baseServo.write(x);             //调整舵机角度
      
      //绘制雷达扫描线
      int f = x - 4; 
      ucg.setColor(0, 255, 0);
      ucg.drawLine(Xcent, base, scanline*cos(radians(f))+Xcent,base - scanline*sin(radians(f)));
      f+=2;
      ucg.setColor(0, 128, 0);
      ucg.drawLine(Xcent, base, scanline*cos(radians(f))+Xcent,base - scanline*sin(radians(f)));
      f+=2;
      ucg.setColor(0, 0, 0);
      ucg.drawLine(Xcent, base, scanline*cos(radians(f))+Xcent,base - scanline*sin(radians(f)));
      ucg.setColor(0,200, 0);
      //测距
      sensor.startMeasurement();
      if (sensor.isReady()) {
        distance = sensor.getDistance();
      }
     
      //根据测得距离在对应位置画点
      if (distance < 100)  // 0-1米内，用红色表示
      {
        ucg.setColor(255,0,0);
        ucg.drawDisc(distance*cos(radians(x))+Xcent,-distance*sin(radians(x))+base, 1, UCG_DRAW_ALL);
      }
      else if (distance < 200)  // 1-2米内，用黄色表示
      {
        ucg.setColor(255,255,0);
        ucg.drawDisc(distance*cos(radians(x))+Xcent,-distance*sin(radians(x))+base, 1, UCG_DRAW_ALL);
      }
      else  // 超过2米，用蓝色表示在边缘区域
      {
        ucg.setColor(0,0,255);
        ucg.drawDisc(116*cos(radians(x))+Xcent,-116*sin(radians(x))+base, 1, UCG_DRAW_ALL);
      }
    
           
      //调试代码，输出角度和测距值  
      Serial.print(x); 
      Serial.print("    ,   ");
      Serial.println(distance); 
     

      if (x > 70 and x < 110)  fix_font();  //扫描线和数字重合时，重绘数字
 
      ucg.setColor(0,155,  0);
      ucg.setPrintPos(0,126);
      ucg.print("DEG: "); 
      ucg.setPrintPos(24,126);
      ucg.print(x);
      ucg.print("  ");
      ucg.setPrintPos(125,126);
      ucg.print("  ");
      ucg.print(distance);
      ucg.print("cm  "); 
      
  }
  //ucg.clearScreen();  //清屏 如果arduino供电不足，可能会引起白屏（显示信号中断）可以用 cls();函数代替 ucg.clearScreen(); 
  delay(50);
  cls();   //如有频繁白屏情况，可以使用该函数 。或者增加外部供电
 
  fix(); 
  fix_font();          //重绘屏幕背景元素
  
  for (int  x=1; x < 176; x+=2){     
      baseServo.write(x);             //调整舵机角度
      
      //绘制雷达扫描线
      int f = x + 4;
      ucg.setColor(0, 255, 0);
      ucg.drawLine(Xcent, base, scanline*cos(radians(f))+Xcent,base - scanline*sin(radians(f)));
      f-=2;
      ucg.setColor(0, 128, 0);
      ucg.drawLine(Xcent, base, scanline*cos(radians(f))+Xcent,base - scanline*sin(radians(f)));
      f-=2;
      ucg.setColor(0, 0, 0);
      ucg.drawLine(Xcent, base, scanline*cos(radians(f))+Xcent,base - scanline*sin(radians(f)));
      ucg.setColor(0, 200, 0);
      //测距
      sensor.startMeasurement();
      if (sensor.isReady())
      {
        distance = sensor.getDistance()/10;
      }

      //根据测得距离在对应位置画点
      if (distance < 100)
      {
        ucg.setColor(255,0,0);
        ucg.drawDisc(distance*cos(radians(x))+Xcent,-distance*sin(radians(x))+base, 1, UCG_DRAW_ALL);
      }
      else
      { //超过1米以上的，用黄色画在边缘区域示意
        ucg.setColor(255,255,0);
        ucg.drawDisc(116*cos(radians(x))+Xcent,-116*sin(radians(x))+base, 1, UCG_DRAW_ALL);
      }
           
      //调试代码，输出角度和测距值  
      Serial.print(x); 
      Serial.print("    ,   ");
      Serial.println(distance); 
     
      if (x > 70 and x < 110)  fix_font();  //扫描线和数字重合时，重绘数字
      
      ucg.setColor(0,155,  0);
      ucg.setPrintPos(0,126);
      ucg.print("DEG: "); 
      ucg.setPrintPos(24,126);
      ucg.print(x);
      ucg.print("   ");
      ucg.setPrintPos(125,126);
      ucg.print(" ");
      ucg.print(distance);
      ucg.print("cm   "); 
  
  }
 //ucg.clearScreen(); //
 delay(50);
 cls();


}
