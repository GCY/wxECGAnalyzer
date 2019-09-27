#ifndef __DEFINE__
#define __DEFINE__

#include <stdint.h>

//const uint32_t SAMPLING_RATE = 1000;
const uint32_t SAMPLING_RATE = 360;

typedef struct
{
   float value;
   int32_t index;
}SignalPoint;

enum
{
   NOTQRS,		/* not-QRS (not a getann/putann code) */
   NORMAL,		/* normal beat */
   LBBB,		/* left bundle branch block beat */
   RBBB,		/* right bundle branch block beat */
   ABERR,		/* aberrated atrial premature beat */
   PVC,		/* premature ventricular contraction */
   FUSION,		/* fusion of ventricular and normal beat */
   NPC,		/* nodal (junctional) premature beat */
   APC,		/* atrial premature contraction */
   SVPB,		/* premature or ectopic supraventricular beat */
   VESC,		/* ventricular escape beat */
   NESC,		/* nodal (junctional) escape beat */
   PACE,		/* paced beat */
   UNKNOWN,	/* unclassifiable beat */
   NOISE,		/* signal quality change */
   ARFCT		/* isolated QRS-like artifact */   
};

#endif
