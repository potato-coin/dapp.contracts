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

struct [[ potato::table, potato::contract("dice") ]] st_bet_stats
{
   uint64_t id;
   name player;
   asset amount;
   uint8_t roll_under;
   time_point_sec bet_time;
   uint64_t my_clear_id;

   uint64_t primary_key() const { return id; }
   POTATOLIB_SERIALIZE(st_bet_stats, (id)(player)(amount)(roll_under)(bet_time)(my_clear_id))
};
typedef potato::multi_index<"bet_stats"_n, st_bet_stats> stats;

struct [[ potato::table, potato::contract("dice") ]] st_bet_clearing_stats
{
   uint64_t id;
   uint64_t bet_id;
   asset amount;
   name player;
   uint8_t roll_under;
   uint8_t random_roll;
   time_point_sec bet_time;

   uint64_t primary_key() const { return id; }
   POTATOLIB_SERIALIZE(st_bet_clearing_stats, (id)(bet_id)(amount)(player)(roll_under)(random_roll)(bet_time))
};
typedef potato::multi_index<"bet_clear"_n, st_bet_clearing_stats> clearing_stats;

struct [[ potato::table, potato::contract("dice") ]] st_mybet_clearing_stats
{
   uint64_t id;
   uint64_t bet_id;
   asset amount;
   uint8_t roll_under;
   uint8_t random_roll;
   time_point_sec bet_time;

   uint64_t primary_key() const { return id; }
   uint64_t get_secondary() const { return bet_id; }
   POTATOLIB_SERIALIZE(st_mybet_clearing_stats, (id)(bet_id)(amount)(roll_under)(random_roll)(bet_time))
};
typedef potato::multi_index<"my_clear"_n, st_mybet_clearing_stats,
   indexed_by< "betid"_n, const_mem_fun<st_mybet_clearing_stats, uint64_t, &st_mybet_clearing_stats::get_secondary> >
   > my_clearing_stats;

class[[potato::contract("dice")]] dice : public contract
{
public:
   using contract::contract;

   static constexpr potato::name active_permission{"active"_n};
   static constexpr potato::name token_account{"pc.token"_n};
   static constexpr potato::name transfer_action{"transfer"_n};
   static constexpr potato::name dice_edge{"dice.edge"_n};
   static constexpr potato::symbol token_symbol{symbol_code("POC"), 4};

   dice(name receiver, name code, datastream<const char *> ds);

   //下注
   [[potato::action]] void bet(const name player, const asset amount, const uint32_t rollnum);

   [[potato::action]] void resolvebet(const uint64_t bet_id);

   // 退回下注
   [[potato::action]] void refundbet(const name player, const uint64_t bet_id);

   //
   [[potato::action]] void test();
   [[potato::action]] void test2(const name player);

private:
   stats bet_stats;
   clearing_stats bet_clearing_stats;

   uint64_t get_token_balance(const name player, const symbol_code &token_type) const;
   uint64_t get_payout_mult_times10000(const uint32_t roll_under, const uint64_t house_edge_times_10000) const;
   uint64_t get_max_win() const;
};

} // namespace potato
