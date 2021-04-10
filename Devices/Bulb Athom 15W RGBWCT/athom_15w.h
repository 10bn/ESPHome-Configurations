#pragma once

#include "esphome.h"

class RGBWCT : public Component, public LightOutput {
    public:
        RGBWCT(FloatOutput *cw_white_color, FloatOutput *brightness, FloatOutput *red, FloatOutput *green, FloatOutput *blue){
            cw_white_color_ = cw_white_color;
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
            traits.set_supports_rgb_white_value(true); 
            traits.set_supports_color_interlock(false);
            traits.set_supports_color_temperature(true);
            traits.set_min_mireds(this->cold_white_temperature_);
            traits.set_max_mireds(this->warm_white_temperature_);
            return traits;
        }


        void write_state(LightState *state) override {
            float brightness, cwhite, wwhite, red, green, blue, white;

            //read values
            state->current_values_as_rgbw(&red, &green, &blue, &white, this->color_interlock_);
            state->current_values_as_cwww(&cwhite, &wwhite);
            state->current_values_as_brightness(&brightness);

            //write brightness and color temp for white light
            if((red+green+blue)/brightness>2.9){
                this->brightness_->set_level(brightness);
                this->cw_white_color_->set_level(cwhite/brightness);
            }

            //write brightness and color temp for color light
            //this->brightness_->set_level(0); //No color white mix
            else{
                this->brightness_->set_level(white); //mix white into color
                this->cw_white_color_->set_level(0);
            }

            //debugging
            ESP_LOGD("custom", "R:%f G:%f B:%f Br: %f, cw: %f, ww: %f, CI: %d, W: %f" , 
            red, green, blue, brightness, cwhite, wwhite , this->color_interlock_, white);

            //write color values
            this->red_->set_level(red);
            this->green_->set_level(green);
            this->blue_->set_level(blue);
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
