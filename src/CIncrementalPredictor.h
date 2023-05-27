/////////////////////////////////////////////////////////////////////////////
//
// CIncrementalPredictor.h
//
// Rémi Coulom
//
// September, 2007
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CIncrementalPredictor_Declared
#define CIncrementalPredictor_Declared

#include <string>
#include <vector>

namespace whr {
 
 class CGame;
 
 class CIncrementalPredictor // ipred
 {
  private: ///////////////////////////////////////////////////////////////////
   std::vector<float *> vParameter;
   std::vector<std::string> vParameterName;

  protected: /////////////////////////////////////////////////////////////////
   void AddParameter(const std::string &sName, float &f)
   {
    vParameter.push_back(&f);
    vParameterName.push_back(sName);
   }

  public: ////////////////////////////////////////////////////////////////////
   //
   // Parameter management
   //
   int GetParameters() {return int(vParameter.size());}
   float &GetParameter(int i) {return *vParameter[i];}
   const std::string &GetParameterName(int i) {return vParameterName[i];}

   //
   // Reset: must be called if parameters are changed
   //
   virtual void Reset() {}

   //
   // Add one game result (games must be added in chronological order)
   //
   virtual void AddGame(const CGame &game) = 0;
 
   //
   // Compute the probability that Black wins
   //
   virtual float Predict(int Handicap,
                         int Komi,
                         int Black,
                         int White,
                         int Day) = 0;
 
   //
   // Get rating and its variance
   //
   virtual float GetRating(int Player) = 0;
   virtual float GetVariance(int Player) = 0;
 
   //
   // Virtual destructor
   //
   virtual ~CIncrementalPredictor() {}
 };
}

#endif
