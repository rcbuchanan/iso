#include "Reg.hh"

//const char *isoReg::SNAKE_PART_PATH = "C:\\Users\\rc\\Documents\\iso\\Models\\snakepart.x";
const char *isoReg::SNAKE_PART_PATH = "./Models/segment.x";
//const char *isoReg::WOULD_BOUNDING_BOX_PATH = "C:\\Users\\rc\\Documents\\iso\\Models\\worldbound.x";
const char *isoReg::WORLD_BOUNDING_BOX_PATH = "./Models/worldbound.x";
const char *isoReg::STAR_PATH = "./Models/star.x";
const u32 isoReg::SNAKE_LENGTH = 8;

const f32 isoReg::DEATH_ANIMATION_DURATION = 500.0;
const f32 isoReg::BIRTH_ANIMATION_DURATION = 500.0;

const f32 isoReg::GRID_DIST = 2;
const f32 isoReg::CLOSE_DIST = 0.01;

const char *isoReg::SKYBOX_PATH[6] = {
	"./Models/skybox/1.png",
	"./Models/skybox/2.png",
	"./Models/skybox/3.png",
	"./Models/skybox/4.png",
	"./Models/skybox/5.png",
	"./Models/skybox/6.png"
};

const char *isoReg::LEVEL_PATHS[] = {
	"./Levels/level1.xml"
};