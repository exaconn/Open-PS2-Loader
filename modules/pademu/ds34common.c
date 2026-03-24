#include "ds34common.h"

#include "loadcore.h"
#include "sysclib.h"

void translate_pad_guitar(const struct ds3guitarreport *in, struct ds2report *out, uint8_t guitar_hero_format)
{
    out->RightStickX = 0x7F;
    out->RightStickY = 0x7F;
    out->LeftStickX = 0x7F;
    out->LeftStickY = -(in->Whammy);

    static const u8 dpad_mapping[] = {
        (DS2ButtonUp),
        (DS2ButtonUp | DS2ButtonRight),
        (DS2ButtonRight),
        (DS2ButtonDown | DS2ButtonRight),
        (DS2ButtonDown),
        (DS2ButtonDown | DS2ButtonLeft),
        (DS2ButtonLeft),
        (DS2ButtonUp | DS2ButtonLeft),
        0,
    };

    u8 dpad = in->Dpad > DS4DpadDirectionReleased ? DS4DpadDirectionReleased : in->Dpad;

    out->nButtonStateL = ~(in->Select | in->Start << 3 | dpad_mapping[dpad]);

    out->nLeft = 0;
    out->nL2 = 1;

    if (guitar_hero_format) {
        // GH PS3 Guitars swap Yellow and Blue
        // Interestingly, it is only GH PS3 Guitars that do this, all the other instruments including GH Drums don't have this swapped.
        out->nButtonStateH = ~(in->Green << 1 | in->Blue << 4 | in->Red << 5 | in->Yellow << 6 | in->Orange << 7);
        if (in->AccelX > 512 || in->AccelX < 432) {
            out->nL2 = 0;
        }
    } else {
        out->nButtonStateH = ~(in->StarPower | in->Green << 1 | in->Yellow << 4 | in->Red << 5 | in->Blue << 6 | in->Orange << 7);
    }
}

// Anti-deadzone here
uint8_t process_stick(uint8_t in)
{
    int adz = 38;
    int out = (int)in;
   
    if (out >= 128) {
        out += adz;
    } else {
        out -= adz;
    }
   
    if (out < 0) out = 0;
    if (out > 255) out = 255;

    return (uint8_t)out;
}
    
void translate_pad_ds3(const struct ds3report *in, struct ds2report *out, u8 pressure_emu)
{
    out->nButtonStateL = ~in->ButtonStateL;
    out->nButtonStateH = ~(in->L1 | in->R1 << 1 | in->L2 << 2 | in->R2 << 3 | in->Triangle << 4 | in->Circle << 5 | in->Cross << 6 | in->Square << 7);

    out->RightStickX = in->RightStickX;
    out->RightStickY = in->RightStickY;
    // Anti-deadzone here
    out->LeftStickX = in->LeftStickX;//process_stick(in->LeftStickX);
    out->LeftStickY = in->LeftStickY;

    if (pressure_emu) { // needs emulating pressure buttons
        out->PressureRight = in->Right * 255;
        out->PressureLeft = in->Left * 255;
        out->PressureUp = in->Up * 255;
        out->PressureDown = in->Down * 255;

        out->PressureTriangle = in->Triangle * 255;
        out->PressureCircle = in->Circle * 255;
        out->PressureCross = in->Cross * 255;
        out->PressureSquare = in->Square * 255;

        out->PressureL1 = in->L2 * 255;
        out->PressureR1 = in->R2 * 255;
        out->PressureL2 = in->L1 * 255;
        out->PressureR2 = in->R1 * 255;
    } else {
        out->PressureRight = in->PressureRight;
        out->PressureLeft = in->PressureLeft;
        out->PressureUp = in->PressureUp;
        out->PressureDown = in->PressureDown;

        out->PressureTriangle = in->PressureTriangle;
        out->PressureCircle = in->PressureCircle;
        out->PressureCross = in->PressureCross;
        out->PressureSquare = in->PressureSquare;

        out->PressureL1 = in->PressureL2;
        out->PressureR1 = in->PressureR2;
        out->PressureL2 = in->PressureL1;
        out->PressureR2 = in->PressureR1;
    }
}

void translate_pad_ds4(const struct ds4report *in, struct ds2report *out, u8 have_touchpad)
{
    static const u8 dpad_mapping[] = {
        (DS2ButtonUp),
        (DS2ButtonUp | DS2ButtonRight),
        (DS2ButtonRight),
        (DS2ButtonDown | DS2ButtonRight),
        (DS2ButtonDown),
        (DS2ButtonDown | DS2ButtonLeft),
        (DS2ButtonLeft),
        (DS2ButtonUp | DS2ButtonLeft),
        0,
    };

    u8
        dpad = in->Dpad > DS4DpadDirectionReleased ? DS4DpadDirectionReleased : in->Dpad, // Just in case an unexpected value appears
        select = in->Share,
        start = in->Option;

    if (have_touchpad && in->TPad) {
        if (!in->nFinger1Active) {
            if (in->Finger1X < 960)
                select = 1;
            else
                start = 1;
        }

        if (!in->nFinger2Active) {
            if (in->Finger2X < 960)
                select = 1;
            else
                start = 1;
        }
    }

    out->nButtonStateL = ~(select | in->L3 << 1 | in->R3 << 2 | start << 3 | dpad_mapping[dpad]);
    out->nButtonStateH = ~(in->L2 | in->R2 << 1 | in->L1 << 2 | in->R1 << 3 | in->Triangle << 4 | in->Circle << 5 | in->Cross << 6 | in->Square << 7);

    out->RightStickX = in->RightStickX;
    out->RightStickY = in->RightStickY;
    out->LeftStickX = in->LeftStickX;
    out->LeftStickY = in->LeftStickY;

    out->PressureRight = out->nRight ? 0 : 255;
    out->PressureLeft = out->nLeft ? 0 : 255;
    out->PressureUp = out->nUp ? 0 : 255;
    out->PressureDown = out->nDown ? 0 : 255;

    out->PressureTriangle = in->Triangle * 255;
    out->PressureCircle = in->Circle * 255;
    out->PressureCross = in->Cross * 255;
    out->PressureSquare = in->Square * 255;

    out->PressureL1 = in->L1 * 255;
    out->PressureR1 = in->R1 * 255;
    out->PressureL2 = in->PressureL2;
    out->PressureR2 = in->PressureR2;
}

void translate_wheel_df(const struct dfreport *in, struct ds2report *out)
{
    /*
    static const u8 dpad_mapping[] = {
        (DS2ButtonUp),
        (DS2ButtonUp | DS2ButtonRight),
        (DS2ButtonRight),
        (DS2ButtonDown | DS2ButtonRight),
        (DS2ButtonDown),
        (DS2ButtonDown | DS2ButtonLeft),
        (DS2ButtonLeft),
        (DS2ButtonUp | DS2ButtonLeft),
        0,
    };

    out->nButtonStateL = ~(in->select | in->L3 << 1 | in->R3 << 2 | in->start << 3 | dpad_mapping[in->hat]);
    */
    
    u8 up = 0, down = 0, left = 0, right = 0;

    switch (in->hat) {
        case 0:
            up = 1;
            break;
        case 1:
            up = 1;
            right = 1;
            break;
        case 2:
            right = 1;
            break;
        case 3:
            down = 1;
            right = 1;
            break;
        case 4:
            down = 1;
            break;
        case 5:
            down = 1;
            left = 1;
            break;
        case 6:
            left = 1;
            break;
        case 7:
            up = 1;
            left = 1;
            break;
        case 8:
            up = 0;
            down = 0;
            left = 0;
            right = 0;
            break;
    }
            
    out->nButtonStateL = ~(in->select | in->L3 << 1 | in->R3 << 2 | in->start << 3 | up << 4 | right << 5 | down << 6 | left << 7);            
    out->nButtonStateH = ~(in->L2 | in->R2 << 1 | in->L1 << 2 | in->R1 << 3 | in->triangle << 4 | in->circle << 5 | in->cross << 6 | in->square << 7);

    out->RightStickX = 0x7F; // neutral
    out->RightStickY = 0x7F; // neutral
    out->LeftStickX = process_stick(in->wheel >> 2); // in->wheel >> 2; // 8bit conversion
    out->LeftStickY = 0x7F; // neutral
    
    out->PressureRight = right * 255;
    out->PressureLeft = left * 255;
    out->PressureUp = up * 255;
    out->PressureDown = down * 255;

    /*
    out->PressureRight = out->nRight ? 0 : 255;
    out->PressureLeft = out->nLeft ? 0 : 255;
    out->PressureUp = out->nUp ? 0 : 255;
    out->PressureDown = out->nDown ? 0 : 255;
    */

    out->PressureTriangle = in->triangle * 255;
    out->PressureCircle = in->circle * 255;
    out->PressureCross = 255 - in->gasPedal; // inverted
    out->PressureSquare = 255 - in->brakePedal; // inverted

    out->PressureL1 = in->L1 * 255;
    out->PressureR1 = in->R1 * 255;
    out->PressureL2 = in->L2 * 255;
    out->PressureR2 = in->R2 * 255;
}

void translate_wheel_negcon(const struct dfreport *in, struct ncreport *out)
{
    uint8_t up = 0, down = 0, left = 0, right = 0;

    switch (in->hat) {
        case 0:
            up = 1;
            break;
        case 1:
            up = 1;
            right = 1;
            break;
        case 2:
            right = 1;
            break;
        case 3:
            down = 1;
            right = 1;
            break;
        case 4:
            down = 1;
            break;
        case 5:
            down = 1;
            left = 1;
            break;
        case 6:
            left = 1;
            break;
        case 7:
            up = 1;
            left = 1;
            break;
        case 8:
            up = 0;
            down = 0;
            left = 0;
            right = 0;
            break;
            
    }
            
    out->nButtonStateL = ~(in->start << 3 | up << 4 | right << 5 | down << 6 | left << 7);
    out->nButtonStateH = ~(in->R1 << 3 | in->triangle << 4 | in->circle << 5);
               
    out->SteeringAxis = in->wheel >> 2; // 8bit conversion
    out->AnalogI = 255 - in->gasPedal; // inverted
    out->AnalogII = 255 - in->brakePedal; // inverted
    out->AnalogL = in->L1 * 255;
}
