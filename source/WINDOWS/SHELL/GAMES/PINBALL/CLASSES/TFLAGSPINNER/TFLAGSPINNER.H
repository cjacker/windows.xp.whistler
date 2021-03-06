//
// Copyright (c) 2000  Microsoft Corporation
//

#include "../../pinball.h"

#ifndef PINBALL_TFLAGSPINNER_H
#define PINBALL_TFLAGSPINNER_H

/* 105 */
struct TFlagSpinner;

void TFlagSpinner::SpinTimer(int, void*); // idb
void __thiscall TFlagSpinner::NextFrame(TFlagSpinner* this); // idb
int __thiscall TFlagSpinner::Message(TFlagSpinner* this, int, float); // idb
int __thiscall TFlagSpinner::get_scoring(TFlagSpinner* this, int); // idb
void __thiscall TFlagSpinner::put_scoring(TFlagSpinner* this, int, int); // idb
void __thiscall TFlagSpinner::Collision(TFlagSpinner* this, struct TBall*, struct vector_type*, struct vector_type*, float, struct TEdgeSegment*); // idb
TFlagSpinner* __thiscall TFlagSpinner::TFlagSpinner(TFlagSpinner* this, struct TPinballTable* a2, int a3);

void* TFlagSpinner::vftable = &TFlagSpinner::Message; // weak

#endif //PINBALL_TFLAGSPINNER_H
