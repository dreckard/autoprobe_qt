/****************************************************************/
/*                     Richard A. Deist                         */
/*           Major Analytical Instrumentation Center            */
/*                   University of Florida                      */
/*                            2011                              */
/****************************************************************/
#ifndef CONSTANTS_H
#define CONSTANTS_H

const unsigned int RENDERMODE_WIREFRAME = 0;
const unsigned int RENDERMODE_SPEC1 = 1;
const unsigned int RENDERMODE_SPEC2 = 2;
const unsigned int RENDERMODE_SPEC3 = 3;
const unsigned int RENDERMODE_SPEC4 = 4;
const unsigned int RENDERMODE_CURRENT = 5;

const unsigned int RENDERMODE_FIRST_IMG = RENDERMODE_SPEC1;
const unsigned int RENDERMODE_LAST_IMG = RENDERMODE_CURRENT;
const unsigned int RENDERMODE_NUM_IMG = ((RENDERMODE_LAST_IMG - RENDERMODE_FIRST_IMG) + 1);

const unsigned int NUM_SPECTROMETERS = 4;

const float BEAM_MIN = 0.001f; //Beam current (nA) below this is considered zero
const unsigned int NUM_ZERO_PTS = 3; //Num consecutive points to trigger zero current abort

const unsigned short AUTOSAVE_OFF = 0;
const unsigned short AUTOSAVE_LINE = 1;
const unsigned short AUTOSAVE_POINT = 2;

const char CFG_FILE[] = "AutoProbe.cfg";

//This should be incremented when compatibility breaking changes are made
const char FILE_FMT_REV = 5; //Current
const char LOWEST_FMT_REV = 3; //Oldest still supported

const int g_iMagicNumber = 0xDECADE + 'M' + 'A' + 'I' + 'C';

#endif // CONSTANTS_H
