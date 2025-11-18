/*******************************************************************************
 *
 * Copyright (c) 2023
 * Lumi, JSC.
 * All Rights Reserved
 *
 *
 * Description: Get light intensity value (Lux & FootCandles) from Light dependent Resistor (implementation)
 *
 * Author: Quentin Comte-Gaz
 *
 * Last Changed By:  $Author: HoangNH $
 * Revision:         $Revision: 1.0 $
 * Last Changed:     $Date: 02/08/2023 $
 *
 ******************************************************************************/
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include "LightDependentResistor.h"
#include <math.h>
/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/
#define OTHER_RESISTOR         		3300 //!< Resistor used for the voltage divider, unit ohms
#define ADC_RESOLUTION_BITS    		12 // Default ADC resolution
#define SMOOTHING_HISTORY_SIZE 		10 // Default linear smooth (if used)

/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/
static float _mult_value; //!< Multiplication parameter in "I[lux]=mult_value/(R[Ω]^pow_value)" expression
static float _pow_value; //!< Power parameter in "I[lux]=mult_value/(R[Ω]^pow_value)" expression
static bool _photocell_on_ground = false; //!< Photocell is connected to +5V/3.3V (false) or GND (true) ?
float _smoothing_sum; //!< (smoothing only) Current sum of valid values of \v _smoothing_history_values
uint32_t _smoothing_history_size; //!< (smoothing only) Size of the table of values
uint32_t _smoothing_history_next; //!< (smoothing only) Next value to get/replace
float _smoothing_history_values[100]; //!< (smoothing only) All valid values (in lux) in a table of \v _smoothing_history_size values maximum (oldest value will be replaced by a new one if table is full)

/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/
/******************************************************************************/
/*                            PRIVATE FUNCTIONS                               */
/******************************************************************************/
/******************************************************************************/
/*                            EXPORTED FUNCTIONS                              */
/******************************************************************************/

void LDR_init(ePhotoCellDeviceType typeDevice)
{
    _photocell_on_ground = false;
    _smoothing_history_size = SMOOTHING_HISTORY_SIZE;

    switch (typeDevice)
    {
        case GL5516:
            _mult_value = 29634400;
            _pow_value = 1.6689;
            break;
        case GL5537_1:
            _mult_value = 32435800;
            _pow_value = 1.4899;
            break;
        case GL5537_2:
            _mult_value = 2801820;
            _pow_value = 1.1772;
            break;
        case GL5539:
            _mult_value = 208510000;
            _pow_value = 1.4850;
            break;
        case GL5549:
            _mult_value = 44682100;
            _pow_value = 1.2750;
            break;
        case GL5528:
        default:
            _mult_value = 32017200;
            _pow_value = 1.5832;
    }

    if (_smoothing_history_size > (sizeof(_smoothing_history_values) / sizeof(_smoothing_history_values[0]))) {
        _smoothing_history_size = (sizeof(_smoothing_history_values) / sizeof(_smoothing_history_values[0]));
    }

    for (uint32_t i = 0 ; i < _smoothing_history_size; i++)
    {
        // We initialize the values as impossible value (lux can't be negative)
        _smoothing_history_values[i] = -1.0f;
    }
}

void LDR_setPhotocellPositionOnGround(bool on_ground)
{
    _photocell_on_ground = on_ground;
}

void LDR_updatePhotocellParameters(float mult_value, float pow_value)
{
    _mult_value = mult_value;
    _pow_value = pow_value;
}

float LDR_luxToFootCandles(float intensity_in_lux)
{
    return intensity_in_lux / 10.764;
}

float LDR_footCandlesToLux(float intensity_in_footcandles)
{
    return 10.764 * intensity_in_footcandles;
}

float LDR_rawAnalogValueToLux(uint16_t raw_analog_value)
{
    unsigned long photocell_resistor;

    if (pow(2, ADC_RESOLUTION_BITS) == raw_analog_value)
    {
        raw_analog_value--;
    }

    float ratio = ((float)pow(2, ADC_RESOLUTION_BITS) / (float)raw_analog_value) - 1;
    if (_photocell_on_ground)
    {
        photocell_resistor = OTHER_RESISTOR / ratio;
    }
    else
    {
        photocell_resistor = OTHER_RESISTOR * ratio;
    }

    return _mult_value / (float)pow(photocell_resistor, _pow_value);
}

float LDR_getCurrentLux(uint16_t rawAnalogValue)
{
    return LDR_rawAnalogValueToLux(rawAnalogValue);
}

float LDR_getCurrentFootCandles(uint16_t rawAnalogValue)
{
    return LDR_luxToFootCandles(LDR_getCurrentLux(rawAnalogValue));
}

float LDR_getSmoothedLux(uint16_t rawAnalogValue)
{
    float sumResult = 0;

    if (_smoothing_history_size == 0)
    {
        // Smoothing disabled, return current value.
        sumResult = LDR_getCurrentLux(rawAnalogValue);
    }
    else
    {
        if (_smoothing_history_values[_smoothing_history_next] < -0.1f)
        {
            // Smoothing enabled but not all values are filled yet
            // (Let's fill one more)
            _smoothing_history_values[_smoothing_history_next] = LDR_getCurrentLux(rawAnalogValue);
            _smoothing_sum += _smoothing_history_values[_smoothing_history_next];

            if (_smoothing_history_next < _smoothing_history_size - 1)
            {
                // Still not all buffers filled
                _smoothing_history_next++;
                sumResult = _smoothing_sum / _smoothing_history_next;
            }
            else
            {
                // All buffers filled now, start regular operation
                _smoothing_history_next = 0;
                sumResult =  _smoothing_sum / _smoothing_history_size;
            }
        }
        else
        {
            // Smoothing enabled and buffer filled previously.
            // => Regular operation from now on:

            // Replace previous value by the new one (from buffer and sum)
            _smoothing_sum -= _smoothing_history_values[_smoothing_history_next];
            _smoothing_history_values[_smoothing_history_next] = LDR_getCurrentLux(rawAnalogValue);
            _smoothing_sum += _smoothing_history_values[_smoothing_history_next];

            // Update next value tu acquire
            _smoothing_history_next = (_smoothing_history_next < _smoothing_history_size - 1) ? _smoothing_history_next + 1 : 0;

            sumResult = _smoothing_sum / _smoothing_history_size;
        }
    }

    return sumResult;
}

float LDR_getSmoothedFootCandles(uint16_t rawAnalogValue)
{
    return LDR_luxToFootCandles(LDR_getSmoothedLux(rawAnalogValue));
}

/* END_FILE */
