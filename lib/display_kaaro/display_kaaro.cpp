#include <display_kaaro.h>
#include "Parola_Fonts_data.h"
#include <Font_Data.h>
uint32_t stoi(String payload, int len);

uint8_t scrollSpeed = 25;
textEffect_t scrollEffect = PA_SCROLL_LEFT;
textPosition_t scrollAlign = PA_LEFT;
uint16_t scrollPause = 2000;


MD_Parola ParolaDisplay = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

DigitalIconDisplay::DigitalIconDisplay() 
{
    counter_value = 0;
    display_state = BOOTING;
    last_frame_display_state = BOOTING;
    display_type = DOTMATRIX;
    display_mode = STANDARD_MODE;
}

void DigitalIconDisplay::stripe()
// Demonstrates animation of a diagonal stripe moving across the display
// with points plotted outside the display region ignored.
{

  const uint16_t maxCol = MAX_DEVICES*ROW_SIZE;
  const uint8_t	stripeWidth = 10;

  Serial.println("\nEach individually by row then col");
  mx.clear();

  for (uint16_t col=0; col<maxCol + ROW_SIZE + stripeWidth; col++)
  {
    for (uint8_t row=0; row < ROW_SIZE; row++)
    {
      mx.setPoint(row, col-row, true);
      mx.setPoint(row, col-row - stripeWidth, false);
    }
    delay(100);
  }
}

void DigitalIconDisplay::spiral()
// setPoint() used to draw a spiral across the whole display
{
  Serial.println("\nSpiral in");
  int  rmin = 0, rmax = ROW_SIZE-1;
  int  cmin = 0, cmax = (COL_SIZE*MAX_DEVICES)-1;

  mx.clear();
  while ((rmax > rmin) && (cmax > cmin))
  {
    // do row
    for (int i=cmin; i<=cmax; i++)
    {
      mx.setPoint(rmin, i, true);
      delay(100/MAX_DEVICES);
    }
    rmin++;

    // do column
    for (uint8_t i=rmin; i<=rmax; i++)
    {
      mx.setPoint(i, cmax, true);
      delay(100/MAX_DEVICES);
    }
    cmax--;

    // do row
    for (int i=cmax; i>=cmin; i--)
    {
      mx.setPoint(rmax, i, true);
      delay(100/MAX_DEVICES);
    }
    rmax--;

    // do column
    for (uint8_t i=rmax; i>=rmin; i--)
    {
      mx.setPoint(i, cmin, true);
      delay(100/MAX_DEVICES);
    }
    cmin++;
  }
}

void DigitalIconDisplay::bounce()
// Animation of a bouncing ball
{
  const int minC = 0;
  const int maxC = mx.getColumnCount()-1;
  const int minR = 0;
  const int maxR = ROW_SIZE-1;

  int  nCounter = 0;

  int  r = 0, c = 2;
  int8_t dR = 1, dC = 1;	// delta row and column

  Serial.println("\nBouncing ball");
  mx.clear();

  while (nCounter++ < 200)
  {
    mx.setPoint(r, c, false);
    r += dR;
    c += dC;
    mx.setPoint(r, c, true);
    delay(100/2);

    if ((r == minR) || (r == maxR))
      dR = -dR;
    if ((c == minC) || (c == maxC))
      dC = -dC;
  }
}

void DigitalIconDisplay::scrollText(char *p)
{
  uint8_t charWidth;
  uint8_t cBuf[8];  // this should be ok for all built-in fonts

  Serial.println("\nScrolling text");
  mx.clear();

  while (*p != '\0')
  {
    charWidth = mx.getChar(*p++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);

    for (uint8_t i=0; i<=charWidth; i++)	// allow space between characters
    {
      mx.transform(MD_MAX72XX::TSL);
      if (i < charWidth)
        mx.setColumn(0, cBuf[i]);
      delay(100);
    }
  }
}
void DigitalIconDisplay::transformation1()
// Demonstrates the use of transform() to move bitmaps on the display
// In this case a user defined bitmap is created and animated.
{
    int  DELAYTIME  =  100;
  uint8_t arrow[COL_SIZE] =
  {
//   0b00000000,
//   0b00000010,
//   0b00000010,
//   0b00001010,
//   0b00001010,
//   0b00101010,
//   0b00101010,
//   0b10101010
    0b00001000,
    0b00011100,
    0b00111110,
    0b01111111,
    0b00011100,
    0b00011100,
    0b00111110,
    0b00000000
  };

  MD_MAX72XX::transformType_t  t[] =
  {
    MD_MAX72XX::TSL, MD_MAX72XX::TSL, MD_MAX72XX::TSL, MD_MAX72XX::TSL,
    MD_MAX72XX::TSL, MD_MAX72XX::TSL, MD_MAX72XX::TSL, MD_MAX72XX::TSL,
    MD_MAX72XX::TSL, MD_MAX72XX::TSL, MD_MAX72XX::TSL, MD_MAX72XX::TSL,
    MD_MAX72XX::TSL, MD_MAX72XX::TSL, MD_MAX72XX::TSL, MD_MAX72XX::TSL,
    MD_MAX72XX::TFLR,
    MD_MAX72XX::TSR, MD_MAX72XX::TSR, MD_MAX72XX::TSR, MD_MAX72XX::TSR,
    MD_MAX72XX::TSR, MD_MAX72XX::TSR, MD_MAX72XX::TSR, MD_MAX72XX::TSR,
    MD_MAX72XX::TSR, MD_MAX72XX::TSR, MD_MAX72XX::TSR, MD_MAX72XX::TSR,
    MD_MAX72XX::TSR, MD_MAX72XX::TSR, MD_MAX72XX::TSR, MD_MAX72XX::TSR,
    MD_MAX72XX::TRC,
    MD_MAX72XX::TSD, MD_MAX72XX::TSD, MD_MAX72XX::TSD, MD_MAX72XX::TSD,
    MD_MAX72XX::TSD, MD_MAX72XX::TSD, MD_MAX72XX::TSD, MD_MAX72XX::TSD,
    MD_MAX72XX::TFUD,
    MD_MAX72XX::TSU, MD_MAX72XX::TSU, MD_MAX72XX::TSU, MD_MAX72XX::TSU,
    MD_MAX72XX::TSU, MD_MAX72XX::TSU, MD_MAX72XX::TSU, MD_MAX72XX::TSU,
    MD_MAX72XX::TINV,
    MD_MAX72XX::TRC, MD_MAX72XX::TRC, MD_MAX72XX::TRC, MD_MAX72XX::TRC,
    MD_MAX72XX::TINV
  };

  Serial.println("\nTransformation1");
  mx.clear();

  // use the arrow bitmap
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
  for (uint8_t j=0; j<mx.getDeviceCount(); j++)
    mx.setBuffer(((j+1)*COL_SIZE)-1, COL_SIZE, arrow);
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  delay(DELAYTIME);

  // run through the transformations
  mx.control(MD_MAX72XX::WRAPAROUND, MD_MAX72XX::ON);
  for (uint8_t i=0; i<(sizeof(t)/sizeof(t[0])); i++)
  {
    mx.transform(t[i]);
    delay(DELAYTIME*4);
  }
  mx.control(MD_MAX72XX::WRAPAROUND, MD_MAX72XX::OFF);
}
void DigitalIconDisplay::cross()
// Combination of setRow() and setColumn() with user controlled
// display updates to ensure concurrent changes.
{
    int  DELAYTIME  =  300;  // in milliseconds 
  Serial.println("\nMoving cross");
  mx.clear();
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  // diagonally down the display R to L
  for (uint8_t i=0; i<ROW_SIZE; i++)
  {
    for (uint8_t j=0; j<MAX_DEVICES; j++)
    {
      mx.setColumn(j, i, 0xff);
      mx.setRow(j, i, 0xff);
    }
    mx.update();
    delay(DELAYTIME);
    for (uint8_t j=0; j<MAX_DEVICES; j++)
    {
      mx.setColumn(j, i, 0x00);
      mx.setRow(j, i, 0x00);
    }
  }

  // moving up the display on the R
  for (int8_t i=ROW_SIZE-1; i>=0; i--)
  {
    for (uint8_t j=0; j<MAX_DEVICES; j++)
    {
      mx.setColumn(j, i, 0xff);
      mx.setRow(j, ROW_SIZE-1, 0xff);
    }
    mx.update();
    delay(DELAYTIME);
    for (uint8_t j=0; j<MAX_DEVICES; j++)
    {
      mx.setColumn(j, i, 0x00);
      mx.setRow(j, ROW_SIZE-1, 0x00);
    }
  }

  // diagonally up the display L to R
  for (uint8_t i=0; i<ROW_SIZE; i++)
  {
    for (uint8_t j=0; j<MAX_DEVICES; j++)
    {
      mx.setColumn(j, i, 0xff);
      mx.setRow(j, ROW_SIZE-1-i, 0xff);
    }
    mx.update();
    delay(DELAYTIME);
    for (uint8_t j=0; j<MAX_DEVICES; j++)
    {
      mx.setColumn(j, i, 0x00);
      mx.setRow(j, ROW_SIZE-1-i, 0x00);
    }
  }
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}
void DigitalIconDisplay::bullseye()
// Demonstrate the use of buffer based repeated patterns
// across all devices.
{
   int  DELAYTIME  =  100;  // in milliseconds 
  Serial.println("\nBullseye");
  mx.clear();
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  for (uint8_t n=0; n<3; n++)
  {
    byte  b = 0xff;
    int   i = 0;

    while (b != 0x00)
    {
      for (uint8_t j=0; j<MAX_DEVICES+1; j++)
      {
        mx.setRow(j, i, b);
        mx.setColumn(j, i, b);
        mx.setRow(j, ROW_SIZE-1-i, b);
        mx.setColumn(j, COL_SIZE-1-i, b);
      }
      mx.update();
      delay(3*DELAYTIME);
      for (uint8_t j=0; j<MAX_DEVICES+1; j++)
      {
        mx.setRow(j, i, 0);
        mx.setColumn(j, i, 0);
        mx.setRow(j, ROW_SIZE-1-i, 0);
        mx.setColumn(j, COL_SIZE-1-i, 0);
      }

      bitClear(b, i);
      bitClear(b, 7-i);
      i++;
    }

    while (b != 0xff)
    {
      for (uint8_t j=0; j<MAX_DEVICES+1; j++)
      {
        mx.setRow(j, i, b);
        mx.setColumn(j, i, b);
        mx.setRow(j, ROW_SIZE-1-i, b);
        mx.setColumn(j, COL_SIZE-1-i, b);
      }
      mx.update();
      delay(3*DELAYTIME);
      for (uint8_t j=0; j<MAX_DEVICES+1; j++)
      {
        mx.setRow(j, i, 0);
        mx.setColumn(j, i, 0);
        mx.setRow(j, ROW_SIZE-1-i, 0);
        mx.setColumn(j, COL_SIZE-1-i, 0);
      }

      i--;
      bitSet(b, i);
      bitSet(b, 7-i);
    }
  }

  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}




int DigitalIconDisplay::setupIcon()
{
    ParolaDisplay.begin();
    ParolaDisplay.setInvert(false);
    ParolaDisplay.setIntensity(15);

    mx.begin();
    mx.control(MD_MAX72XX::INTENSITY, 10);
        // delay(100);
        // mx.setPoint(3,8,false);
        // mx.setPoint(4,8,false);
        // mx.setPoint(3,9,false);
        // mx.setPoint(4,9,false);

    ParolaDisplay.displayText(BOOT_TEXT, PA_CENTER, 70, 100, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
        delay(1000);
    ParolaDisplay.displayAnimate();
    return 1;
}

int DigitalIconDisplay::updateDisplayState(di_display_states updated_state)
{
    last_frame_display_state = display_state;

    display_state = updated_state;
    switch (updated_state)
    {
    case BOOTING:
        /* code */

        break;

    case CONNECTING:
        /* code */
        break;

    case COUNTER:
        /* code */
        break;

    case MESSAGE:
        timeout_start_millis = millis();
        text_refresh = true;
        /* code */
        break;

    case ERROR:
        timeout_start_millis = millis();
        /* code */
        break;

    default:
        break;
    }
    return 1;
}

int DigitalIconDisplay::updateDisplayMode(di_display_mode updated_mode)
{
    switch (updated_mode)
    {
    case STANDARD_MODE:
        /* code */
        break;
    case VINTAGE_MODE:
        /* code */
        break;
    case SCROLL_MODE:
        /* code */
        break;
    case BETA_MODE:
        /* code */
        break;
    default:
        break;
    }
    return 1;
}

int DigitalIconDisplay::showCustomMessage(char *custom_text)
{
    strcpy(display_text, custom_text);
    updateDisplayState(MESSAGE);
    return 1;
}
int DigitalIconDisplay::showCustomMessage(String custom_text) 
{
    char buff[100];
    custom_text.toCharArray(buff, 100);
    return showCustomMessage(buff);
}

int DigitalIconDisplay::updateCounterValue(uint32_t new_counter_value)
{
    target_counter_value = new_counter_value;
    return 1;
}

int DigitalIconDisplay::updateCounterValue(String new_counter_value, bool isString) {
    ParolaDisplay.setFont(numeric7Seg);
    ParolaDisplay.print(String(new_counter_value).c_str());
    return updateCounterValue(
        stoi(new_counter_value, new_counter_value.length())
    ); 
}

void DigitalIconDisplay::loop()
{
    ParolaDisplay.displayAnimate();
    switch (display_state)
    {
    case BOOTING:
        /* code */
        break;

    case CONNECTING:
        /* code */
        break;

    case COUNTER:
        {
        /* code */
        uint32_t toShow = current_counter_value;

        if (target_counter_value > current_counter_value)
        {
            toShow += (target_counter_value - current_counter_value) / 2 + 1;
            current_counter_value = toShow;
        }
        else
        {
            toShow = target_counter_value;
        }

        counter_value = toShow;
        refreshScreenWithCounter();
        }
        break;

    case MESSAGE:
    {
        unsigned long current_millis = millis();
        if (current_millis - timeout_start_millis > TIMEOUT_PERIOD)
        {
            updateDisplayState(COUNTER);
        }
        if (text_refresh)
        {
            text_refresh = false;
            refreshScreenWithText();
        }

    }
        break;

    case ERROR:
    {
        unsigned long current_millis = millis();
        if (current_millis - timeout_start_millis > TIMEOUT_PERIOD)
        {
            updateDisplayState(COUNTER);
        }

    }
        break;

    default:
        break;
    }
    last_frame_display_state = display_state;
}

int DigitalIconDisplay::refreshScreenWithText()
{
    ParolaDisplay.setFont(ExtASCII);
    delay(100);
    ParolaDisplay.displayText(display_text, PA_CENTER, 70, 1000, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    ParolaDisplay.displayAnimate();
    return 1;
}
int DigitalIconDisplay::refreshScreenWithCounter()
{
    ParolaDisplay.setFont(numeric7Seg);
    // P.displayText(msg.c_str(), PA_CENTER, 70, 1000, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
    ParolaDisplay.print(String(counter_value).c_str());
    ParolaDisplay.displayAnimate();

    return 1;
}

uint32_t stoi(String payload, int len)
{
  uint32_t i = 0;
  uint32_t result = 0;
  for (i = 0; i < len; i++)
  {
    result *= 10;
    result += (char)payload[i] - '0';
  }

  return result;
}
