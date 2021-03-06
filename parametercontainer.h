#ifndef PARAMETERCONTAINER_H
#define PARAMETERCONTAINER_H
#include<QString>
#include <QtCore>

class ParameterContainer
{
public:    
    ParameterContainer();

    virtual void updateParamValue()=0;
    virtual void displayParamValue()=0;
    virtual void setValueInHardware(int)=0;
    virtual void setValueInHardware(double)=0;
    virtual void setValueInHardware(std::string)=0;
    virtual void setValueInHardware(bool)=0;

    bool getParameterAvailable() const;
    void setParameterAvailable(bool value);

private:
    bool parameterAvailable=true;



    //signals:
    //    void updateAllParametersSignal();
};

#endif // CONTAINER_H
