#include "Math.h"

namespace MowerMath {

    float normalizeAngle(float angle) {
        // Reducer vinkel til 0-360 interval
        while (angle < 0.0) {
            angle += 360.0;
        }
        while (angle >= 360.0) {
            angle -= 360.0;
        }
        return angle;
    }

    float angleDifference(float angle1, float angle2) {
        // Beregn difference og normaliser til -180 til 180
        float diff = angle2 - angle1;

        // Normaliser til -180 til 180
        while (diff < -180.0) {
            diff += 360.0;
        }
        while (diff > 180.0) {
            diff -= 360.0;
        }

        return diff;
    }

    float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
        // Lineær mapping mellem to intervaller
        return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    int constrainInt(int value, int min, int max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }

    float constrainFloat(float value, float min, float max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }

    float distance(float x1, float y1, float x2, float y2) {
        float dx = x2 - x1;
        float dy = y2 - y1;
        return sqrt(dx * dx + dy * dy);
    }

    float degreesToRadians(float degrees) {
        return degrees * PI / 180.0;
    }

    float radiansToDegrees(float radians) {
        return radians * 180.0 / PI;
    }

    float lowPassFilter(float currentValue, float newValue, float alpha) {
        // Alpha mellem 0 og 1
        alpha = constrainFloat(alpha, 0.0, 1.0);
        return alpha * newValue + (1.0 - alpha) * currentValue;
    }

    float lerp(float start, float end, float t) {
        // Lineær interpolation
        t = constrainFloat(t, 0.0, 1.0);
        return start + t * (end - start);
    }

    bool inRange(float value, float min, float max) {
        return (value >= min && value <= max);
    }

    int sign(float value) {
        if (value > 0) return 1;
        if (value < 0) return -1;
        return 0;
    }

    float absoluteValue(float value) {
        return (value < 0) ? -value : value;
    }

}
