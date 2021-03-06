#ifndef Sensor_H_
#define Sensor_H_

class PSerial;

class Sensor
{
public:

    enum ButtonState {
        BUTTON_PREV,
        BUTTON_NEXT,
        BUTTON_INFO,
        BUTTON_ZOOM,
        NUM_BUTTONS
    };


    enum PollState {
        kPollSuccess,
        kPollError
    };

    virtual ~Sensor();

    static Sensor* Create();

    PollState Poll();

    bool HasChanged();

    float ReadPan();

    float ReadTilt();

    bool IsButtonPressed(ButtonState button);

private:

    explicit Sensor(PSerial* device);

    void Parse(char* buf);

    PSerial* fDevice;

    float fPan;
    float fTilt;

    bool  fCPan;
    bool  fCTilt;
    bool  fCButton;

    int   fButton[NUM_BUTTONS];

};

#endif
