#include "TSeagulls.h"
#include "..\common_h\rands.h"

//#pragma warning (disable : 4244)

//--------------------------------------------------------------------
TSeagulls::TSeagulls() : enabled(true), frightened(false), count(0)
{
}

//--------------------------------------------------------------------
TSeagulls::~TSeagulls()
{
    _CORE_API->DeleteEntity(seagullModel);
}

//--------------------------------------------------------------------
void TSeagulls::LoadSettings()
{
    INIFILE *ini = _CORE_API->fio->OpenIniFile(ANIMALS_INI_FILENAME);
    if (!ini)
    {
        countAdd = SEAGULL_ADD_COUNT;
        maxRadius = SEAGULL_MAX_RADIUS;
        maxAngleSpeed = SEAGULL_MAX_SPEED;
        maxHeight = SEAGULL_MAX_HEIGHT;
        maxCircleTime = SEAGULL_MAX_CIRCLE_TIME;
        farChoiceChance = SEAGULL_LONG_DISTANCE_CHANCE;
        maxDistance = SEAGULL_DISTANCE;
        relaxTime = SEAGULL_RELAX_TIME;
        screamTime = SEAGULL_SCREAM_TIME;
        strcpy(screamFilename, ANIMALS_SEAGULLS_SCREAM_FILENAME);

        return;
    }

    maxRadius = ini->GetFloat(ANIMALS_SEAGULLS_SECTION, "radius", SEAGULL_MAX_RADIUS);
    maxAngleSpeed = ini->GetFloat(ANIMALS_SEAGULLS_SECTION, "angle speed", SEAGULL_MAX_SPEED);
    maxDistance = ini->GetFloat(ANIMALS_SEAGULLS_SECTION, "distance", SEAGULL_DISTANCE);
    maxHeight = ini->GetFloat(ANIMALS_SEAGULLS_SECTION, "height", SEAGULL_MAX_HEIGHT);
    maxCircleTime = ini->GetLong(ANIMALS_SEAGULLS_SECTION, "circle time", SEAGULL_MAX_CIRCLE_TIME);
    farChoiceChance = ini->GetLong(ANIMALS_SEAGULLS_SECTION, "far choice", SEAGULL_LONG_DISTANCE_CHANCE);
    relaxTime = ini->GetLong(ANIMALS_SEAGULLS_SECTION, "relax time", SEAGULL_RELAX_TIME);
    screamTime = ini->GetLong(ANIMALS_SEAGULLS_SECTION, "scream time", SEAGULL_SCREAM_TIME);
    countAdd = ini->GetLong(ANIMALS_SEAGULLS_SECTION, "add count", SEAGULL_ADD_COUNT);
    ini->ReadString(ANIMALS_SEAGULLS_SECTION, "scream file", screamFilename, 256, ANIMALS_SEAGULLS_SCREAM_FILENAME);

    delete ini;
}

//--------------------------------------------------------------------
void TSeagulls::Init()
{
    startY = 0.f;
    LoadSettings();

    renderService = (VDX8RENDER *)_CORE_API->CreateService("dx8render");
    soundService = (VSoundService *)_CORE_API->CreateService("SoundService");

    if (!renderService)
        SE_THROW_MSG("!Seagulls: No service: dx8render");
    // if(!soundService)
    //	SE_THROW_MSG("!Seagulls: No service: sound");

    _CORE_API->CreateEntity(&seagullModel, "MODELR");
    _CORE_API->Send_Message(seagullModel, "ls", MSG_MODEL_LOAD_GEO, ANIMALS_SEAGULL_FILENAME);
}

//--------------------------------------------------------------------
dword TSeagulls::ProcessMessage(long _code, MESSAGE &message)
{
    GUARD(TSeagulls::ProcessMessage)

    dword outValue = 0;

    switch (_code)
    {
    case MSG_ANIMALS_SEAGULLS_SHOW:
        enabled = true;
        break;

    case MSG_ANIMALS_SEAGULLS_HIDE:
        enabled = false;
        count = 0;
        break;

    case MSG_ANIMALS_SEAGULLS_FRIGHTEN:
        Frighten();
        break;

    case MSG_ANIMALS_SEAGULLS_ADD:
        Add(message.Float(), message.Float(), message.Float());
        break;
    }

    return outValue;
    UNGUARD
}

//--------------------------------------------------------------------
void TSeagulls::Execute(dword _dTime)
{
    GUARD(ANIMALS::Execute)

    if (!enabled)
        return;

    // <relax>
    if (frightened)
    {
        frightenTime -= _dTime;
        if (frightenTime <= 0)
        {
            frightened = false;
            screamTime <<= 1;
            for (int i = 0; i < count; i++)
            {
                seagulls[i].va /= 2.0f;
                // seagulls[i].circleTimePassed += _dTime;
            }
        }
    }

    // <all_movements>
    for (int i = 0; i < count; i++)
    {
        // <scream>
        if (seagulls[i].screamTime > 0)
            seagulls[i].screamTime -= _dTime;
        else
        {
            seagulls[i].screamTime = (rand() % (screamTime >> 1)) + (screamTime >> 1);
            CVECTOR pos((float)(seagulls[i].center.x + sin(seagulls[i].a) * seagulls[i].radius),
                        (float)(seagulls[i].center.z + cos(seagulls[i].a) * seagulls[i].radius),
                        (float)(seagulls[i].height));

            // if(soundService) soundService->SoundPlay(screamFilename, PCM_3D, VOLUME_FX, false, false, true, 0, &pos);
        }

        // <angle_inc>
        seagulls[i].a = fmodf(seagulls[i].a + (seagulls[i].va / 1000000.0f) * _dTime, 2 * PI);

        // <new_circle>
        if (seagulls[i].circleTimePassed < seagulls[i].circleTime)
            seagulls[i].circleTimePassed += _dTime;
        else
        {
            float oldR = seagulls[i].radius;
            seagulls[i].radius = SEAGULL_MIN_RADIUS + rand(maxRadius);
            seagulls[i].circleTimePassed = 0;
            seagulls[i].circleTime = (long)rand((float)maxCircleTime);
            if ((seagulls[i].circleTime) < (maxCircleTime / 20))
                seagulls[i].circleTime = maxCircleTime / 20;
            float sinA = sinf(seagulls[i].a);
            float cosA = cosf(seagulls[i].a);
            float newX1 = seagulls[i].center.x + sinA * (seagulls[i].radius + oldR);
            float newZ1 = seagulls[i].center.z + cosA * (seagulls[i].radius + oldR);
            float newX2 = seagulls[i].center.x + sinA * (oldR - seagulls[i].radius);
            float newZ2 = seagulls[i].center.z + cosA * (oldR - seagulls[i].radius);
            float distance1 = fabsf(cameraPos.x - newX1) + fabsf(cameraPos.z - newZ1);
            float distance2 = fabsf(cameraPos.x - newX2) + fabsf(cameraPos.z - newZ2);
            float oldVa = seagulls[i].va;

            seagulls[i].va *= oldR / seagulls[i].radius;
            float deltaVa = randCentered(maxAngleSpeed / 5.0f);
            if (((seagulls[i].va + deltaVa) * seagulls[i].va) < 0)
                seagulls[i].va -= deltaVa;
            else
                seagulls[i].va += deltaVa;

            float minRadius = maxRadius * maxAngleSpeed / seagulls[i].radius;
            if (fabs(seagulls[i].va) < (minRadius / 2.0f))
            {
                if (seagulls[i].va > 0.0f)
                    seagulls[i].va = minRadius / 2.0f;
                else
                    seagulls[i].va = -minRadius / 2.0f;
            }

            // seagulls[i].height += randCentered(maxHeight / 50.0f);
            if ((distance1 < distance2) && ((rand() % farChoiceChance) == 1))
            {
                seagulls[i].center.x = newX1;
                seagulls[i].center.z = newZ1;
                seagulls[i].va = -seagulls[i].va;
                seagulls[i].a = fmodf(seagulls[i].a + PI, 2 * PI);
            }
            else
            {
                seagulls[i].center.x = newX2;
                seagulls[i].center.z = newZ2;
            }
        }
    }

    UNGUARD
}

//--------------------------------------------------------------------
void TSeagulls::Realize(dword _dTime)
{
    GUARD(ANIMALS::Realize)

    if (!enabled)
        return;

    float persp;
    renderService->GetCamera(cameraPos, cameraAng, persp);
    if (!count)
        Add(cameraPos.x, cameraPos.y, cameraPos.z);

    MODEL *seagull = (MODEL *)_CORE_API->GetEntityPointer(&seagullModel);
    if (!seagull)
        return;

    for (int i = 0; i < count; i++)
    {
        CVECTOR ang, pos;
        ang.x = 0.0f;
        ang.z = 0.0f;
        float angle = seagulls[i].a;
        if (seagulls[i].va > 0.0f)
            ang.y = seagulls[i].a + PI / 2;
        else
            ang.y = seagulls[i].a - PI / 2;
        pos.x = seagulls[i].center.x + sinf(seagulls[i].a) * seagulls[i].radius;
        pos.z = seagulls[i].center.z + cosf(seagulls[i].a) * seagulls[i].radius;
        pos.y = seagulls[i].height;
        seagull->mtx = CMatrix(ang, pos);
        seagull->Realize(_dTime);
    }

    UNGUARD
}

//--------------------------------------------------------------------
void TSeagulls::Frighten()
{
    if (frightened)
        return;

    frightened = true;
    frightenTime = relaxTime;

    for (int i = 0; i < count; i++)
    {
        seagulls[i].va *= 2.0f;
        seagulls[i].screamTime >>= 2;
    }
    screamTime >>= 1;
}

//--------------------------------------------------------------------
void TSeagulls::Add(float _x, float _y, float _z)
{
    float op;

    for (int i = count; i < (count + countAdd); i++)
    {
        seagulls[i].center = CVECTOR(randCentered(maxDistance) + _x, rand(maxHeight) + SEAGULL_MIN_HEIGHT,
                                     randCentered(maxDistance) + _z);
        seagulls[i].radius = randUpper(maxRadius);
        op = randUpper(maxAngleSpeed);
        if (rand() & 0x1)
            seagulls[i].va = op;
        else
            seagulls[i].va = -op;
        seagulls[i].height = startY + seagulls[i].center.y;
        seagulls[i].a = rand(2 * PI);
        seagulls[i].circleTime = rand() % maxCircleTime;
        seagulls[i].screamTime = (rand() % (screamTime >> 3)) << 3;
    }
    count += countAdd;
}
//--------------------------------------------------------------------