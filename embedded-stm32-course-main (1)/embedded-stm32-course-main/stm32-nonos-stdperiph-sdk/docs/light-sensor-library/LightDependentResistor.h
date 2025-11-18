/*******************************************************************************
 *
 * Copyright (c) 2023
 * Lumi, JSC.
 * All Rights Reserved
 *
 *
 * Description: Get light intensity value (Lux & FootCandles) from Light dependent Resistor (LDR) a.k.a. photocell or photoresistor
 *
 * This library is easily usable with most GL55xx photoresistors (at ~25°C).
 *
 * It is also possible to use it with any other photocell (with the right parameters).
 * If you use this library with other photocells, please send me the parameters in
 * order to add them in the list.
 *
 * Schematics:
 *                           ^
 *            _____      ___/___
 *    5V |---|_____|----|__/____|--| GND
 *    or      Other       /
 *   3.3V    Resistor   Photocell
 *
 * Note: By default, the photocell must be on the ground.
 *       It is possible to exchange the position of the photocell and the other resistor
 *       but you will have to call \p setPhotocellPositionOnGround(false).
 *
 * Author: Quentin Comte-Gaz
 *
 * Last Changed By:  $Author: HoangNH $
 * Revision:         $Revision: 1.0 $
 * Last Changed:     $Date: 02/08/2023 $
 *
 ******************************************************************************/
#ifndef _LDR_H_
#define _LDR_H_
/******************************************************************************/
/*                              INCLUDE FILES                                 */
/******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
/******************************************************************************/
/*                     EXPORTED TYPES and DEFINITIONS                         */
/******************************************************************************/
/*!
 * \enum ePhotoCellKind Photocell component
 */
typedef enum ePhotoCellDeviceType {
	GL5516,
	GL5528,
	GL5537_1,
	GL5537_2,
	GL5539,
	GL5549
} ePhotoCellDeviceType;
/******************************************************************************/
/*                              PRIVATE DATA                                  */
/******************************************************************************/
/******************************************************************************/
/*                              EXPORTED DATA                                 */
/******************************************************************************/
/******************************************************************************/
/*                            EXPORTED FUNCTIONS                              */
/******************************************************************************/

/*!
 * \brief LightDependentResistor Initialize the light intensity getter class
 *
 * Even thought some photocells are already defined, it is possible to
 * define your own photocell.
 * The relation between the photocell resistor and the lux intensity can be
 * approximated to I[lux]=mult_value/(R[Ω]^pow_value).
 *
 * Example for GL5528 photocell:
 *   1) Find curve Resistor->Lux intensity: http://cdn.sparkfun.com/datasheets/Sensors/LightImaging/SEN-09088.pdf
 *   2) Get 2 points from the datasheet log curve: log(55000[Ω])->log(1[lux]) and log(3000[Ω])->log(100[lux])
 *   3) Convert those 2 point into a "log linear curve" (with Excel for example): log(R[Ω]) = -0.6316 * log(I[lux]) + 4.7404 (linear)
 *   4) Solve the equation to get I[lux]=mult_value/(R[Ω]^pow_value) approximation (with wolframalpha for example): I[lux] ~= 32017200/R[Ω]^1.5832
 *      https://www.wolframalpha.com/input/?i=log10(x)%3D-0.6316*log10(y)%2B4.7404
 *   5) You just found the 2 parameters: mult_value=32017200 and pow_value=1.5832
 *
 * \param other_resistor (unsigned long) Resistor used for the voltage divider
 * \param mult_value (float) Multiplication parameter in "I[lux]=mult_value/(R[Ω]^pow_value)" expression
 * \param pow_value (float) Power parameter in "I[lux]=mult_value/(R[Ω]^pow_value)" expression
 * \param adc_resolution_bits (unsigned int, optional, default: 12) Number of resolution bits for the ADC pin
 * \param smoothing_history_size (unsigned int, optional, default: 10) Max number of raw values used for \f getSmoothedLux or \f getSmoothedFootCandles
 */
void LDR_init(ePhotoCellDeviceType typeDevice);

/*!
 * \brief getCurrentLux Get light intensity (in lux) from the photocell
 *
 * \return (float) Light intensity (in lux)
 */
float LDR_getCurrentLux(uint16_t rawAnalogValue);

/*!
 * \brief getCurrentFootCandles Get light intensity (in footcandles) from the photocell
 *
 * \return (float) Light intensity (in footcandles)
 */
float LDR_getCurrentFootCandles(uint16_t rawAnalogValue);

/*!
 * \brief rawAnalogValueToLux Convert raw value from photocell sensor into lux
 *
 *  This function is only needed if the sensor MUST NOT be handled by this library...
 *  Else, it is better to directly use \f getCurrentLux that will read sensor value and convert it into lux.
 *
 * \param raw_value (int) Analog value of the photocell sensor (WARNING: This value must be with the same adc resolution as the one in the constructor)
 *
 * \return (float) Light intensity (in lux)
 */
float LDR_rawAnalogValueToLux(uint16_t raw_analog_value);

/*!
 * \brief luxToFootCandles Get footcandles from lux intensity
 *
 * \param intensity_in_lux (float) Intensity in lux
 *
 * \return Footcandles retrieved from \p intensity_in_lux
 */
float LDR_luxToFootCandles(float intensity_in_lux);

/*!
 * \brief footCandlesToLux Get Lux intensity from footcandles
 *
 * \param intensity_in_footcandles (float) Footcandles
 *
 * \return Intensity in lux retrieved from \p intensity_in_footcandles
 */
float LDR_footCandlesToLux(float intensity_in_footcandles);

/*!
 * \brief setPhotocellPositionOnGround Configure the photocell as connected to +5V/3.3V or GND
 *
 * \param on_ground (bool) True if the photocell is connected to GND, else false
 *
 *  True:                    ^
 *            _____      ___/___
 *    5V |---|_____|----|__/____|--| GND
 *    or      Other       /
 *   3.3V    Resistor   Photocell
 *
 *  False:                    ^
 *             _____      ___/___
 *    GND |---|_____|----|__/____|--| 5V
 *            Other        /          or
 *           Resistor   Photocell    3.3V
 */
void LDR_setPhotocellPositionOnGround(bool on_ground);

/*!
 * \brief updatePhotocellParameters Redefine the photocell parameters
 *
 * \param mult_value (float) Multiplication parameter in "I[lux]=mult_value/(R[Ω]^pow_value)" expression
 * \param pow_value (float) Power parameter in "I[lux]=mult_value/(R[Ω]^pow_value)" expression
 */
void LDR_updatePhotocellParameters(float mult_value, float pow_value);

/*!
 * \brief getSmoothedLux Read light intensity (in lux) from the photocell, apply linear smoothing using the number of historic values specified with the constructor.
 *
 * \return (float) Light intensity (in lux) after applying linear smoothing
 */
float LDR_getSmoothedLux(uint16_t rawAnalogValue);

/*!
 * \brief getCurrentFootCandles Read light intensity from the photocell, apply linear smoothing using the number of historic values specified with the constructor, convert to footcandles.
 *
 * \return (float) Light intensity (in footcandles) after applying linear smoothing
 */
float LDR_getSmoothedFootCandles(uint16_t rawAnalogValue);

#endif //_LDR_H_
