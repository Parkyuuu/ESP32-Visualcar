#include <Arduino.h>
#include <TFT_eSPI.h>
#include <string>
// 导入图片
#include "img1.h" 


using namespace std;
int TFTtextNowline = 0;


class Image {
public:
    //需要三个输入 imgData指针 int的图像长宽
    Image(const uint16_t* imgData, int width, int height)
        //初始化成员变量
        : imgData(imgData), imgWidth(width), imgHeight(height), 
          x(0), y(0), scale(1) {}

    void setPosition(int newX, int newY) {
        x = newX;
        y = newY;
    }

      void setScale(float newScale) {
        scale = newScale;
    }

    void draw(TFT_eSPI& tft) {
        int newWidth = imgWidth * scale;
        int newHeight = imgHeight * scale;

        // 使用双线性插值绘制图像
        for (int j = 0; j < newHeight; j++) {
            for (int i = 0; i < newWidth; i++) {
                float srcX = (float)i * imgWidth / newWidth;
                float srcY = (float)j * imgHeight / newHeight;
                int x1 = (int)srcX;
                int y1 = (int)srcY;
                int x2 = min(x1 + 1, imgWidth - 1);
                int y2 = min(y1 + 1, imgHeight - 1);

                uint16_t color1 = pgm_read_word(&imgData[y1 * imgWidth + x1]);
                uint16_t color2 = pgm_read_word(&imgData[y1 * imgWidth + x2]);
                uint16_t color3 = pgm_read_word(&imgData[y2 * imgWidth + x1]);
                uint16_t color4 = pgm_read_word(&imgData[y2 * imgWidth + x2]);

                float xWeight = srcX - x1;
                float yWeight = srcY - y1;

                uint16_t color = 
                    (color1 * (1 - xWeight) * (1 - yWeight) +
                     color2 * xWeight * (1 - yWeight) +
                     color3 * (1 - xWeight) * yWeight +
                     color4 * xWeight * yWeight);

                tft.drawPixel(x + i, y + j, color);
            }
        }
    }

    void clear(TFT_eSPI& tft,uint16_t color) {
        tft.fillRect(x, y, imgWidth * scale, imgHeight * scale , color);
    }


private:
    const uint16_t* imgData;
    int imgWidth;    // 原始的宽
    int imgHeight;   // 原始的高
    int x;           // 位置x
    int y;           // 位置y
    float scale;     // 缩放倍数
};

class Text {
public:
    // 构造函数，接收字符串 大小 文本颜色 背景颜色
    Text(const String& text, int textSize, uint16_t textColor, uint16_t backColor)
        : text(text), textSize(textSize), textColor(textColor), backColor(backColor) {}

    // 绘制文字
    void draw(TFT_eSPI& tft, int posX, int posY) {
        tft.setTextColor(textColor, backColor); // 设置文字颜色和背景色
        tft.setTextSize(textSize); // 设置文字大小
        tft.setCursor(posX, posY); // 设置光标位置
        tft.print(text); // 绘制文字
    }
    // 显示在某一行中 
    void drawLine(TFT_eSPI& tft){
        int outTextIndex;
        int maxLine;
        String outText;
        String tempText;

        //最大行数 分辨率 / 行间距 + 1
        maxLine = ( 240 / (textSize * 8 + 9) + 1);
        maxLine = (int)maxLine;

        if (TFTtextNowline + 1 > maxLine){
            tft.fillScreen(TFT_BLACK);
            TFTtextNowline = 0;
        }

        // 应该显示在的y坐标 TFTtextNowline * 9 表示行间距
        int posY = ((TFTtextNowline) * textSize * 8) + (TFTtextNowline) * 9;
        
        //如果超出了边界
        if ((text.length() * 6 * textSize) > 240){
            outTextIndex = 240 / (6 * textSize);
            outTextIndex = (int)outTextIndex;
            //绘制首行
            tft.setTextColor(textColor, backColor);
            tft.setTextSize(textSize);
            tft.setCursor(0, posY);
            tft.print(text.substring(0,outTextIndex));
            TFTtextNowline ++;
            tempText = text;
            for (int i =0; (tempText.length() * 6 * textSize) > 240; i++){
                if (outTextIndex >= tempText.length()){
                    outText = ""; //如果超出就设置为空
                }
                else{
                    outText = tempText.substring(outTextIndex); //切分字符 获取到超出部分的字符内容
                    tft.setTextColor(textColor, TFT_WHITE); 
                    tft.setTextSize(textSize);
                    tft.setCursor(0, posY);
                    tft.print(text);
                } 
                tempText = outText;
                TFTtextNowline ++;
                textLine ++;

                if (TFTtextNowline + 1 > maxLine){
                    tft.fillScreen(TFT_BLACK);
                    TFTtextNowline = 0;
                }
                posY = ((TFTtextNowline + 1) * textSize * 8) + (TFTtextNowline + 1) * 9; //+1 区别多行间的显示
            }
        }
        else{
            tft.setTextColor(textColor, backColor);
            tft.setTextSize(textSize);
            tft.setCursor(0, posY);
            tft.print(text);
            TFTtextNowline ++;
        }
        
    }
    


    //用于结束显示
    void clear(TFT_eSPI& tft, uint16_t color) {
            tft.fillRect(posX, posY, text.length() * 6 * textSize, 8 * textSize, color);
        }


    String text; // 文本内容
    int posX;    // x 坐标
    int posY;    // y 坐标
    int textSize; // 文本大小
    int textLine; //所占行数
    uint16_t textColor; // 文本颜色
    uint16_t backColor; // 背景颜色
};

TFT_eSPI tft = TFT_eSPI();
void showLine(String text, int testSize, uint16_t textColor, uint16_t backColor){
    Text* textObj = new Text(text,testSize,textColor,backColor);
    textObj->drawLine(tft);
    delete textObj;
}
void setup() {
    /*
    tft屏幕使用的引脚
    #define TFT_MOSI 15 
    #define TFT_SCLK 14
    #define TFT_CS   5
    #define TFT_DC   27
    #define TFT_RST  33
    #define TFT_BL   22
    */

    Serial.begin(115200);
    Serial1.println("begin");
    //初始化tft屏幕
    tft.init();
    tft.fillScreen(TFT_BLACK);
    tft.setSwapBytes(true);
    
    //开机动画
    Image imgObject1(img1, img1_width, img1_height);
    imgObject1.setPosition(0, 0);
    imgObject1.setScale(1);
    imgObject1.draw(tft);
    delay(3000);
    //无用的的特效
    for (int y = 0; y < 240; y++) {
        tft.fillRect(0, y, 240, 1, TFT_BLACK);
        delay(2); 

        
    }
    showLine("Hello world", 2, TFT_WHITE, TFT_GREEN);
    showLine("66666", 5, TFT_WHITE, TFT_GREEN);
    


}

void loop() {


}
