#include "shipsign.h"
#include "shipcommand.h"
#include "shipliga.h"

BIShipSign::BIShipSign(BIShipLiga *pLiga)
{
    m_pShipLiga = pLiga;
    m_apCommand = NEW BIShipCommand(this);
    Assert(m_apCommand);
}

BIShipSign::~BIShipSign()
{
    Release();
}

void BIShipSign::Release()
{
    DELETE(m_apCommand);
}