#pragma once

#include "esphome.h"

class RGBWCT : public Component, public LightOutput {
    public:
        RGBWCT(FloatOutput *cw_white_color, FloatOutput *brightness, FloatOutput *red, FloatOutput *green, FloatOutput *blue){
            cw_white_color_ = cw_white_color;
            brightness_ = brightness;
            red_ = red;
            green_ = green;
            blue_ = blue;
            cold_white_temperature_ = 166;
            warm_white_temperature_ = 333;
            color_interlock_ = true;
        }


        LightTraits get_traits() override {
            auto traits = light::LightTraits();
            traits.set_supports_brightness(true);
            traits.set_supports_rgb(true); 
            traits.set_supports_rgb_white_value(false); 
            traits.set_supports_color_interlock(true);
            traits.set_supports_color_temperature(true);
            traits.set_min_mireds(this->cold_white_temperature_);
            traits.set_max_mireds(this->warm_white_temperature_);
            return traits;
        }

        float max(float v1, float v2){
            if(v1 > v2)
                return v1;
            return v2;
        }

        void write_state(LightState *state) override {
            float brightness, cwhite, wwhite, red, green, blue, white;
            float temp = 0.5;
            //read values
            state->current_values_as_rgbww(&red, &green, &blue, &cwhite, &wwhite, true, true);
            state->current_values_as_brightness(&brightness);

            //adjust for brightness
            float cool_amount = cwhite / brightness;
            float warm_amount = wwhite / brightness;
            float r = red / brightness;
            float g = green / brightness;
            float b = blue / brightness;

            //Calculate V component from HSV (used for transition from White mode to RGB Mode)
            float v = max(max(r, g), b);

            //Calculate temperature
            if(warm_amount < 0.999)
                temp = warm_amount / 2;
            else
                temp = ((1 - cool_amount) / 2) + 0.5;

            //invert temperature
            temp = 1 - temp;

            // only tweak brightness if transitioning from white mode
            if(cwhite + wwhite < 0.01)
                this->brightness_->set_level(0);
            else
                this->brightness_->set_level(brightness * (1 - v));

            if(red + green + blue > 0.01)
            {
                this->red_->set_level(red);
                this->green_->set_level(green);
                this->blue_->set_level(blue);
            }
            else{
                this->red_->set_level(0);
                this->green_->set_level(0);
                this->blue_->set_level(0);
                this->cw_white_color_->set_level(temp);
            }
            
        }


    protected:
        FloatOutput *cw_white_color_;
        FloatOutput *brightness_;
        FloatOutput *red_;
        FloatOutput *green_;
        FloatOutput *blue_;
        float cold_white_temperature_;
        float warm_white_temperature_;
        bool color_interlock_;
};
