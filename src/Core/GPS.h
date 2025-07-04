
#ifndef _GPS_H_
#define _GPS_H_

// GPS.h

#include "Core.h"

using namespace std;

class GPS_DD : public Core {
private:
protected:
public:
  double latitude;
  double longitude;

  GPS_DD();
  GPS_DD(double, double);
  const double getLatitude();
  const double getLongitude();
  double distance(GPS_DD);

  bool operator==(GPS_DD &another);
  bool operator<(GPS_DD &another);

  // Json::Value * dump2JSON(void);
  virtual Json::Value *dump2JSON(void) const override;
  virtual void JSON2Object(const Json::Value *) override;
};

#endif /* _GPS_H_ */
