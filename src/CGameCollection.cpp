/////////////////////////////////////////////////////////////////////////////
//
// CGameCollection.cpp
//
// May, 2005
//
/////////////////////////////////////////////////////////////////////////////
#include "CGameCollection.h"
#include "debug.h"

using namespace whr;

/////////////////////////////////////////////////////////////////////////////
// AddName
/////////////////////////////////////////////////////////////////////////////
int CGameCollection::AddName(const std::string &sName)
{
 std::pair<std::map<std::string, int>::iterator, bool> Result =
  NameMap.insert(std::pair<std::string, int>(sName, int(vName.size())));

 //
 // New player
 //
 if (Result.second)
  vName.push_back(Result.first->first);

 return Result.first->second;
}

/////////////////////////////////////////////////////////////////////////////
// AddGame
/////////////////////////////////////////////////////////////////////////////
void CGameCollection::AddGame(const CGame &game)
{
 if (vGame.size() > 0)
  FATAL(game.GetDay() < vGame[vGame.size()-1].GetDay());
 vGame.push_back(game);
}

/////////////////////////////////////////////////////////////////////////////
// Reset
/////////////////////////////////////////////////////////////////////////////
void CGameCollection::Reset(const CGameCollection &gc)
{
 vName = gc.vName;
 NameMap = gc.NameMap;
 vGame.clear();
}

/////////////////////////////////////////////////////////////////////////////
// Lookup
/////////////////////////////////////////////////////////////////////////////
int CGameCollection::Lookup(const std::string &sName) const
{
 std::map<std::string, int>::const_iterator iName = NameMap.find(sName);

 if (iName == NameMap.end())
  return -1;
 else
 {
  int Player = iName->second;
  if (Player >= GetPlayers())
   return -1;
  else
   return Player;
 }
}

/////////////////////////////////////////////////////////////////////////////
// Max Komi
/////////////////////////////////////////////////////////////////////////////
int CGameCollection::GetMaxKomi() const
{
 int Result = -10000;

 for (int i = int(vGame.size()); --i >= 0;)
 {
  int k = vGame[i].GetKomi();
  if (k > Result)
   Result = k;
 }

 return Result;
}

/////////////////////////////////////////////////////////////////////////////
// Min Komi
/////////////////////////////////////////////////////////////////////////////
int CGameCollection::GetMinKomi() const
{
 int Result = 10000;

 for (int i = int(vGame.size()); --i >= 0;)
 {
  int k = vGame[i].GetKomi();
  if (k < Result)
   Result = k;
 }

 return Result;
}

/////////////////////////////////////////////////////////////////////////////
// Max Handicap
/////////////////////////////////////////////////////////////////////////////
int CGameCollection::GetMaxHandicap() const
{
 int Result = 0;

 for (int i = int(vGame.size()); --i >= 0;)
 {
  int h = vGame[i].GetHandicap();
  if (h > Result)
   Result = h;
 }

 return Result;
}

/////////////////////////////////////////////////////////////////////////////
// RemoveUnconnected
/////////////////////////////////////////////////////////////////////////////
int CGameCollection::RemoveUnconnected(int Player, int Result)
{
 if (Player < 0)
  return Player;

 std::vector<int> vConnected(GetPlayers());
 for (int i = GetPlayers(); --i >= 0;)
  vConnected[i] = 0;

 //
 // Loop to find connected players
 //
 vConnected[Player] = 1;
 while (1)
 {
  int Additions = 0;

  for (int i = GetGames(); --i >= 0;)
  {
   const CGame &game = GetGame(i);
   if (vConnected[game.GetBlack()] &&
       !vConnected[game.GetWhite()] &&
       game.GetResult() == Result)
   {
    vConnected[game.GetWhite()] = 1;
    Additions++;
   }
   else if (!vConnected[game.GetBlack()] &&
            vConnected[game.GetWhite()] &&
            game.GetResult() == 1 - Result)
   {
    vConnected[game.GetBlack()] = 1;
    Additions++;
   }
  }
  
  if (Additions == 0)
   break;
 }

 //
 // Compute translation of player numbers
 //
 std::vector<int> vTranslation(GetPlayers());
 std::vector<int> vReverseTranslation(GetPlayers());

 int Players = 0;
 for (int i = 0; i < GetPlayers(); i++)
 {
  vTranslation[i] = Players;
  vReverseTranslation[Players] = i;
  Players += vConnected[i];
 }

 //
 // Build new vector of player names and update game map
 //
 std::vector<std::string> vNewName(Players);
 for (int i = Players; --i >= 0;)
 {
  vNewName[i] = vName[vReverseTranslation[i]];
  NameMap[vNewName[i]] = i;
 }
 vName = vNewName;

 //
 // Build new vector of games
 //
 std::vector<CGame> vNewGame;
 for (unsigned i = 0; i < vGame.size(); i++)
  if (vConnected[vGame[i].GetBlack()] &&
      vConnected[vGame[i].GetWhite()])
  {
   CGame game = vGame[i];
   game.SetWhite(vTranslation[game.GetWhite()]);
   game.SetBlack(vTranslation[game.GetBlack()]);
   vNewGame.push_back(game);
  }
 vGame = vNewGame;

 return vTranslation[Player]; 
}

/////////////////////////////////////////////////////////////////////////////
// Extract games with given handicap and komi
/////////////////////////////////////////////////////////////////////////////
void CGameCollection::ExtractHandicapKomi(int Handicap,
                                          int Komi,
                                          CGameCollection &gc) const
{
 gc.Reset(*this);

 for (int i = 0; i < GetGames(); i++)
 {
  if (vGame[i].GetHandicap() == Handicap && vGame[i].GetKomi() == Komi)
   gc.AddGame(vGame[i]);
 }
}
