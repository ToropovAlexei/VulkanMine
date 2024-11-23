#version 460

layout(location = 0) in vec3 pos;
layout(location = 1) in uint skyRegion;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 view;
    mat4 projection;
    mat4 projectionView;
    vec3 cameraPosition;
    float dayTime;
} ubo;

layout(location = 0) out vec4 outColor;

const vec3 zenithColorDay = vec3(0.5, 0.7, 1.0);
const vec3 horizonColorDay = vec3(0.85, 0.91, 1.0);

const vec3 zenithColorNight = vec3(0.06, 0.05, 0.09);
const vec3 horizonColorNight = vec3(0.07, 0.06, 0.1);

const vec3 zenithColorSunRise = vec3(0.45, 0.5, 0.7);
const vec3 horizonColorSunRise = vec3(0.9, 0.45, 0.15);

const vec3 zenithColorMoonRise = vec3(0.45, 0.5, 0.7);
const vec3 horizonColorMoonRise = vec3(0.5, 0.45, 0.6);

vec3 getColor() {
    if (skyRegion == 1 || skyRegion == 2) {
        if (ubo.dayTime < 2000) { //6am - 9am sun gets brighter
            return mix(zenithColorMoonRise, zenithColorDay, ubo.dayTime / 2000); 
        }
        if (ubo.dayTime >= 2000 && ubo.dayTime < 10000) { //9am - 3pm sun is brightest
            return zenithColorDay;
        }
        if (ubo.dayTime >= 10000 && ubo.dayTime < 12000) { //3pm - 6pm sun gets dimmer
            return mix(zenithColorDay, zenithColorSunRise, (ubo.dayTime - 10000) / 2000);
        }
        if (ubo.dayTime >= 12000 && ubo.dayTime < 14000) { //6pm - 9pm sun light fades
            return mix(zenithColorSunRise, zenithColorNight, (ubo.dayTime - 12000) / 2000);
        }
        if (ubo.dayTime >= 14000 && ubo.dayTime < 22000) {//9pm - 3am is night
            return zenithColorNight;
        }
        if (ubo.dayTime >= 22000 && ubo.dayTime < 24000) {
            return mix(zenithColorNight, zenithColorMoonRise, (ubo.dayTime - 22000) / 2000);
        }
    }
    else if (skyRegion == 0 || skyRegion == 3) {
        if (ubo.dayTime < 2000) { //6am - 9am sun gets brighter
            return mix(horizonColorMoonRise, horizonColorDay, ubo.dayTime / 2000); 
        }
        if (ubo.dayTime >= 2000 && ubo.dayTime < 10000) { //9am - 3pm sun is brightest
            return horizonColorDay;
        }
        if (ubo.dayTime >= 10000 && ubo.dayTime < 12000) { //3pm - 6pm sun gets dimmer
            return mix(horizonColorDay, horizonColorSunRise, (ubo.dayTime - 10000) / 2000);
        }
        if (ubo.dayTime >= 12000 && ubo.dayTime < 14000) { //6pm - 9pm sun light fades
            return mix(horizonColorSunRise, horizonColorNight, (ubo.dayTime - 12000) / 2000);
        }
        if (ubo.dayTime >= 14000 && ubo.dayTime < 22000) {//9pm - 3am is night
            return horizonColorNight;
        }
        if(ubo.dayTime >= 22000 && ubo.dayTime < 24000){
            return mix(horizonColorNight, horizonColorMoonRise, (ubo.dayTime - 22000) / 2000);
        }
    }
    else if (skyRegion == 5 || skyRegion == 6) {
        if (ubo.dayTime < 2000){ //6am - 9am sun gets brighter
            return mix(zenithColorSunRise, zenithColorDay, ubo.dayTime / 2000); 
        }
        if (ubo.dayTime >= 2000 && ubo.dayTime < 10000) { //9am - 3pm sun is brightest
            return zenithColorDay;
        }
        if (ubo.dayTime >= 10000 && ubo.dayTime < 12000) { //3pm - 6pm sun gets dimmer
            return mix(zenithColorDay, zenithColorMoonRise, (ubo.dayTime - 10000) / 2000);
        }
        if (ubo.dayTime >= 12000 && ubo.dayTime < 14000) { //6pm - 9pm sun light fades
            return mix(zenithColorMoonRise, zenithColorNight, (ubo.dayTime - 12000) / 2000);
        }
        if (ubo.dayTime >= 14000 && ubo.dayTime < 22000) {//9pm - 3am is night
            return zenithColorNight;
        }
        if (ubo.dayTime >= 22000 && ubo.dayTime < 24000) {
            return mix(zenithColorNight, zenithColorSunRise, (ubo.dayTime - 22000) / 2000);
        }
    } else if (skyRegion == 4 || skyRegion == 7) {
        if (ubo.dayTime < 2000) { //6am - 9am sun gets brighter
            return mix(horizonColorSunRise, horizonColorDay, ubo.dayTime / 2000); 
        }
        if (ubo.dayTime >= 2000 && ubo.dayTime < 10000) { //9am - 3pm sun is brightest
            return horizonColorDay;
        }
        if (ubo.dayTime >= 10000 && ubo.dayTime < 12000) { //3pm - 6pm sun gets dimmer
            return mix(horizonColorDay, horizonColorMoonRise, (ubo.dayTime - 10000) / 2000);
        }
        if (ubo.dayTime >= 12000 && ubo.dayTime < 14000) { //6pm - 9pm sun light fades
            return mix(horizonColorMoonRise, horizonColorNight, (ubo.dayTime - 12000) / 2000);
        }
        if (ubo.dayTime >= 14000 && ubo.dayTime < 22000) {//9pm - 3am is night
            return horizonColorNight;
        }
        if (ubo.dayTime >= 22000 && ubo.dayTime < 24000) {
            return mix(horizonColorNight, horizonColorSunRise, (ubo.dayTime - 22000) / 2000);
        }
    }
}

void main() {
	gl_Position = ubo.projectionView * vec4(pos + ubo.cameraPosition, 1.0);
	outColor = vec4(getColor(), 1.0);
}