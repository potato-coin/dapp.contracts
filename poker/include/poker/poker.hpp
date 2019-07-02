/**
 *  @file
 *  @copyright defined in potato/LICENSE
 */
#pragma once

#include <potatolib/asset.hpp>
#include <potatolib/potato.hpp>
#include <potatolib/action.hpp>
#include <potatolib/name.hpp>
#include <potatolib/symbol.hpp>
#include <potatolib/transaction.h>
#include <potatolib/crypto.hpp>
#include <potatolib/time.hpp>

#include <string>

namespace potato
{

using std::string;

struct [[ potato::table, potato::contract("poker") ]] st_bet_stats
{
   uint64_t id;
   name player;
   asset amount;
   std::vector<uint8_t> card_under;
   time_point_sec bet_time;
   uint64_t my_clear_id;

   uint64_t primary_key() const { return id; }
   POTATOLIB_SERIALIZE(st_bet_stats, (id)(player)(amount)(card_under)(bet_time)(my_clear_id))
};
typedef potato::multi_index<"bet_stats"_n, st_bet_stats> stats;

struct [[ potato::table, potato::contract("poker") ]] st_bet_clearing_stats
{
   uint64_t id;
   uint64_t bet_id;
   asset amount;
   name player;
   std::vector<uint8_t> card_under;
   uint8_t random_card;
   time_point_sec bet_time;

   uint64_t primary_key() const { return id; }
   POTATOLIB_SERIALIZE(st_bet_clearing_stats, (id)(bet_id)(amount)(player)(card_under)(random_card)(bet_time))
};
typedef potato::multi_index<"bet_clear"_n, st_bet_clearing_stats> clearing_stats;

struct [[ potato::table, potato::contract("poker") ]] st_mybet_clearing_stats
{
   uint64_t id;
   uint64_t bet_id;
   asset amount;
   std::vector<uint8_t> card_under;
   uint8_t random_card;
   time_point_sec bet_time;

   uint64_t primary_key() const { return id; }
   uint64_t get_secondary() const { return bet_id; }
   POTATOLIB_SERIALIZE(st_mybet_clearing_stats, (id)(bet_id)(amount)(card_under)(random_card)(bet_time))
};
typedef potato::multi_index<"my_clear"_n, st_mybet_clearing_stats,
   indexed_by< "betid"_n, const_mem_fun<st_mybet_clearing_stats, uint64_t, &st_mybet_clearing_stats::get_secondary> >
   > my_clearing_stats;

class[[potato::contract("poker")]] poker : public contract
{
public:
   using contract::contract;

   static constexpr potato::name active_permission{"active"_n};
   static constexpr potato::name token_account{"pc.token"_n};
   static constexpr potato::name transfer_action{"transfer"_n};
   static constexpr potato::name poker_edge{"poker.edge"_n};
   static constexpr potato::symbol token_symbol{symbol_code("POC"), 4};

   poker(name receiver, name code, datastream<const char *> ds);

   //下注
   [[potato::action]]
   void bet(const name player, const asset amount, const std::vector<uint8_t>& rollnum);

   [[potato::action]]
   void resolvebet(const uint64_t betid);

   // 退回下注
   [[potato::action]]
   void refundbet(const name player, const uint64_t bet_id);

   // //
   // [[potato::action]] void test();
   // [[potato::action]] void test2(const name player);

private:
   stats bet_stats;
   clearing_stats bet_clearing_stats;

   asset get_token_balance(const name player, const symbol_code &token_type) const;
   asset get_max_win() const;

   static constexpr const float odds[] = {12.8050F, 6.3375F, 4.2033F, 3.12F, 2.47F, 2.0475F, 1.7457F, 1.5194F, 1.3433F, 1.2064F, 1.0932F, 1.0010F};
};

} // namespace potato
