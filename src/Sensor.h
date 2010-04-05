
class Sensor
{
public:
    
    enum ButtonState {
        kButtonPreview,
        kButtonNext,
        kButtonInfo,
        kButtonZoom
    };
    
    
    enum PollState {
        kPollSuccess,
        kPollError
    };
    
    virtual ~Sensor();
          
    static Sensor* Create();
    
    PollState Poll();
    
    bool HasChanged();
    
    float ReadPos();

    float ReadTilt();
    
    bool IsButtonPressed(ButtonState button);
        
private:
    explicit Sensor();
};