#ifndef MATH_UTILS_H
#define MATH_UTILS_H

#include <Arduino.h>

/**
 * MowerMath namespace - Matematiske hjælpefunktioner
 *
 * Indeholder diverse matematiske utility funktioner til navigation
 * og sensor beregninger.
 */
namespace MowerMath {

    /**
     * Normaliserer vinkel til 0-360 grader interval
     * @param angle Vinkel at normalisere
     * @return Normaliseret vinkel (0-360)
     */
    float normalizeAngle(float angle);

    /**
     * Beregner korteste vinkel difference mellem to vinkler
     * @param angle1 Første vinkel (0-360)
     * @param angle2 Anden vinkel (0-360)
     * @return Vinkel difference (-180 til 180)
     */
    float angleDifference(float angle1, float angle2);

    /**
     * Mapper en float værdi fra ét interval til et andet
     * @param x Værdi at mappe
     * @param in_min Input minimum
     * @param in_max Input maximum
     * @param out_min Output minimum
     * @param out_max Output maximum
     * @return Mappet værdi
     */
    float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);

    /**
     * Begrænser en integer værdi til et interval
     * @param value Værdi at begrænse
     * @param min Minimum værdi
     * @param max Maximum værdi
     * @return Begrænset værdi
     */
    int constrainInt(int value, int min, int max);

    /**
     * Begrænser en float værdi til et interval
     * @param value Værdi at begrænse
     * @param min Minimum værdi
     * @param max Maximum værdi
     * @return Begrænset værdi
     */
    float constrainFloat(float value, float min, float max);

    /**
     * Beregner afstand mellem to punkter
     * @param x1 Første punkt X
     * @param y1 Første punkt Y
     * @param x2 Andet punkt X
     * @param y2 Andet punkt Y
     * @return Afstand mellem punkterne
     */
    float distance(float x1, float y1, float x2, float y2);

    /**
     * Konverterer grader til radianer
     * @param degrees Grader
     * @return Radianer
     */
    float degreesToRadians(float degrees);

    /**
     * Konverterer radianer til grader
     * @param radians Radianer
     * @return Grader
     */
    float radiansToDegrees(float radians);

    /**
     * Simpel low-pass filter
     * @param currentValue Nuværende værdi
     * @param newValue Ny værdi
     * @param alpha Filter koefficient (0-1, højere = mere vægt på ny værdi)
     * @return Filtreret værdi
     */
    float lowPassFilter(float currentValue, float newValue, float alpha);

    /**
     * Lineær interpolation mellem to værdier
     * @param start Start værdi
     * @param end Slut værdi
     * @param t Interpolations faktor (0-1)
     * @return Interpoleret værdi
     */
    float lerp(float start, float end, float t);

    /**
     * Tjek om et tal er indenfor et interval (inklusiv)
     * @param value Værdi at tjekke
     * @param min Minimum værdi
     * @param max Maximum værdi
     * @return true hvis indenfor interval
     */
    bool inRange(float value, float min, float max);

    /**
     * Beregner tegn af et tal
     * @param value Værdi
     * @return 1 hvis positiv, -1 hvis negativ, 0 hvis nul
     */
    int sign(float value);

    /**
     * Absolut værdi af et tal
     * @param value Værdi
     * @return Absolut værdi
     */
    float absoluteValue(float value);

}

#endif // MATH_UTILS_H
