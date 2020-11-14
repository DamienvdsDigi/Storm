#ifndef _TSINK_H_
#define _TSINK_H_

#include "..\SoundService\VSoundService.h"
#include "..\common_h\CVECTOR.h"
#include "..\common_h\dx8render.h"
#include "..\common_h\exs.h"
#include "..\common_h\geos.h"
#include "..\common_h\matrix.h"
#include "..\common_h\messages.h"
#include "..\common_h\model.h"
#include "..\common_h\sea_base.h"
#include "TFlotsam.h"
#include "TIVBufferManager.h"
#include "TSinkSplash.h"

///////////////////////////////////////////////////////////////////
// CLASS DEFINITION
///////////////////////////////////////////////////////////////////
class TSink
{
  public:
    TSink();
    virtual ~TSink();

    void Initialize(INIFILE *_ini, IDirect3DDevice9 *_device, SEA_BASE *sea, VDX8RENDER *_renderer);
    void Release();
    void Start(const CVECTOR &_pos, float _radius);
    void Process(dword _dTime);
    void Realize(dword _dTime);
    bool Enabled();

  private:
    bool enabled;

    VDX8RENDER *renderer;
    SEA_BASE *sea;
    long texture;
    TIVBufferManager *ivManager;
    dword time;

    TSinkSplash splashes[MAX_SPLASHES];
    long times[MAX_SPLASHES];
    long ivIndexes[MAX_SPLASHES];
    long flotsamTimes[MAX_FLOTSAMS];
    TFlotsam flotsams[MAX_FLOTSAMS];
    float distortDivider;
    float radius;
    CVECTOR center;
};

#endif