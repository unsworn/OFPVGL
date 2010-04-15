#include "ofSerial.h"
#include "Sensor.h"

//------------------------------------------------------------------------------------------------------------

#define SNAME "Parascope USB Interface"
#define SBAUD 230400

#define SIFMT "%03ld.%01ld:%03ld.%01ld:%02d"

#define SVFMT "%f:%f:%d"

extern struct _GUID GUID_SERENUM_BUS_ENUMERATOR;

//------------------------------------------------------------------------------------------------------------

inline static int
shiftin(unsigned char* buf, int len, int c)
{
    int i;

    --len;

    for (i=0 ; i < len ; i++)
        buf[i] = buf[i+1];

    return (buf[len] = c);

}

//------------------------------------------------------------------------------------------------------------

class PSerial : public ofSerial
{
public:
    PSerial() : ofSerial(), fSelectedPort(NULL)
    {}

    virtual ~PSerial()
    {
        if (NULL != fSelectedPort)
            free( fSelectedPort );
    }

    bool IsPortSelected()
    {
        return (fSelectedPort != NULL);
    }

    const char* GetPort()
    {
        return fSelectedPort;
    }

    bool SelectPort()
    {

        HDEVINFO hDevInfo = NULL;
        SP_DEVINFO_DATA DeviceInterfaceData;
        int i = 0;
        int nPorts = 0;
        DWORD dataType, actualSize = 0;

        unsigned char dataBuf[MAX_PATH + 1];

        char* portName;
        char* end;


        if (NULL != fSelectedPort)
            free(fSelectedPort);


        fSelectedPort = NULL;

        // Search device set
        hDevInfo = SetupDiGetClassDevs((struct _GUID *)&GUID_SERENUM_BUS_ENUMERATOR,0,0,DIGCF_PRESENT);

        if ( hDevInfo ){
            while (TRUE){

                ZeroMemory(&DeviceInterfaceData, sizeof(DeviceInterfaceData));
                DeviceInterfaceData.cbSize = sizeof(DeviceInterfaceData);

                if (!SetupDiEnumDeviceInfo(hDevInfo, i, &DeviceInterfaceData))
                    break;

                if (SetupDiGetDeviceRegistryProperty(hDevInfo,
                                                     &DeviceInterfaceData,
                                                     SPDRP_FRIENDLYNAME,
                                                     &dataType,
                                                     dataBuf,
                                                     sizeof(dataBuf),
                                                     &actualSize))
                {



                    if (strstr((char*)dataBuf, SNAME) != NULL)
                    {
                        portName = strrchr((char*)dataBuf, '(');

                        if (portName == NULL)
                            break;

                        portName++;

                        end = strchr((char*)portName, ')');

                        if (end == NULL)
                            break;

                        (*end) = '\0';

                        fSelectedPort = strdup(portName);

                    }

                }
                i++;
            }
        }

        SetupDiDestroyDeviceInfoList(hDevInfo);

        return IsPortSelected();

    }


private:
    char* fSelectedPort;
};

//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------

Sensor::Sensor(PSerial* device) :
        fDevice(device)
{
}

//------------------------------------------------------------------------------------------------------------

Sensor::~Sensor()
{
    if (NULL != fDevice)
        delete fDevice;
}

//------------------------------------------------------------------------------------------------------------

Sensor*
Sensor::Create()
{
    PSerial* ps = new PSerial();

    if(ps->SelectPort())
    {
        if (!ps->setup(ps->GetPort(), SBAUD))
            printf("Unable to open port %s @%d\n", ps->GetPort(), SBAUD);
        else
            printf("Found Parascope: %s\n", ps->GetPort());
    }

    return new Sensor(ps);
}

//------------------------------------------------------------------------------------------------------------

Sensor::PollState
Sensor::Poll()
{

    int l;

    static unsigned char buf[15];

    while (fDevice->available())
    {
        l= shiftin(buf, 15, fDevice->readByte());

        switch(l)
        {
            case -1:
                return kPollError;
            case 0:
                Parse((char*)buf);
        }
    }

    return kPollSuccess;
}

//------------------------------------------------------------------------------------------------------------

bool
Sensor::HasChanged()
{
    return true;
}

//------------------------------------------------------------------------------------------------------------

float
Sensor::ReadPan()
{
    return fPan;
}

//------------------------------------------------------------------------------------------------------------

float
Sensor::ReadTilt()
{
    return fTilt;
}

//------------------------------------------------------------------------------------------------------------

bool
Sensor::IsButtonPressed(ButtonState button)
{
    if ( button >= kNumButtons)
        return false;

    return (fButton[ button ] != 0);
}

//------------------------------------------------------------------------------------------------------------

void
Sensor::Parse(char* buf)
{
    static float xlast = 0;
    static float ylast = 0;
    static int   blast = 0;

    float xtmp = 0;
    float ytmp = 0;
    int   btmp = 0;

    sscanf(buf, SVFMT, &xtmp, &ytmp, &btmp);

    fButton[ kButtonPrev ] = (btmp&0x01);
    fButton[ kButtonNext ] = (btmp&0x02);
    fButton[ kButtonInfo ] = (btmp&0x04);
    fButton[ kButtonZoom ] = (btmp&0x08);

    fPan  = ytmp;   // pan aorund y-axis
    fTilt = xtmp;   // tilt around x-axis

    xlast = xtmp;
    ylast = ytmp;
    blast = btmp;
}
