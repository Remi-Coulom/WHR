/////////////////////////////////////////////////////////////////////////////
//
// CGameCollection.h
//
// May, 2005
//
/////////////////////////////////////////////////////////////////////////////
#ifndef WHR_CGameCollection_Declared
#define WHR_CGameCollection_Declared

#include <vector>
#include <string>
#include <map>
#include "CGame.h"

namespace whr {
 class CGameCollection // gc
 {
  private: //////////////////////////////////////////////////////////////////
   std::vector<std::string> vName;
   std::map<std::string, int> NameMap;

   std::vector<CGame> vGame;

  public: ///////////////////////////////////////////////////////////////////
   //
   // Name-list management
   //
   int GetPlayers() const {return int(vName.size());}
   int AddName(const std::string &sName); // returns player number
   int Lookup(const std::string &sName) const;
   const std::string &GetName(int PlayerNumber) const
   {
    return vName[PlayerNumber];
   }

   //
   // Game-list management
   //
   int GetGames() const {return int(vGame.size());}
   void AddGame(const CGame &game); // must be added in chronological order
   const CGame &GetGame(int i) const {return vGame[i];}
   void Reset(const CGameCollection &gc); // empty collection with same names

   //
   // Remove unconnected players
   //
   int RemoveUnconnected(int Player, int Result);

   //
   // Extract a subset of games
   //
   void ExtractHandicapKomi(int Handicap, int Komi, CGameCollection &gc) const;

   //
   // Compute some maxes and mins
   //
   int GetMaxKomi() const;
   int GetMinKomi() const;
   int GetMaxHandicap() const;
 };
}

#endif
