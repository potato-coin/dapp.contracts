/**
 *  @file
 *  @copyright defined in potato/LICENSE
 */

#include <pocker/pocker.hpp>
#include <pc.token/pc.token.hpp>
#include <potatolib/print.hpp>
#include <potatolib/transaction.hpp>

namespace potato
{

const uint64_t HOUSEEDGE_times10000 = 200;
const uint64_t HOUSEEDGE_REF_times10000 = 150;
const uint64_t REFERRER_REWARD_times10000 = 50;

pocker::pocker(name receiver, name code, datastream<const char *> ds)
    : contract(receiver, code, ds),
      bet_stats(_self, _self.value),
      bet_clearing_stats(_self, _self.value)
{
}

void pocker::bet(const name player, const asset bet_amount, const uint32_t roll_under)
{
   require_auth(player);
   potato_assert(bet_amount.symbol == token_symbol, "asset symbol error");
   potato_assert(bet_amount.amount >= 0, "Less than minimum limit amount");
   // potato_assert(amount.amount <= get_max_win(), "Exceed maximum limit amount");
   potato_assert(roll_under >= 2 && roll_under <= 96, "Roll must be >= 2, <= 96.");

   uint64_t your_bet_amount = bet_amount.amount;
   uint64_t house_edge = HOUSEEDGE_REF_times10000;

   const uint64_t your_win_amount = (your_bet_amount * get_payout_mult_times10000(roll_under, house_edge) / 10000) - your_bet_amount;
   potato_assert(your_win_amount <= get_max_win(), "Bet less than max");

   auto s = read_transaction(nullptr, 0);
   char *tx = (char *)malloc(s);
   read_transaction(tx, s);
   auto tx_hash = potato::sha256(tx, s).data();

   const uint64_t bet_id = ((uint64_t)tx_hash[0] << 56) + ((uint64_t)tx_hash[1] << 48) + ((uint64_t)tx_hash[2] << 40) + ((uint64_t)tx_hash[3] << 32) + ((uint64_t)tx_hash[4] << 24) + ((uint64_t)tx_hash[5] << 16) + ((uint64_t)tx_hash[6] << 8) + (uint64_t)tx_hash[7];

   INLINE_ACTION_SENDER(potato::token, transfer)
   (
      token_account, {{player, active_permission}},
      {player, _self, asset(bet_amount.amount, token_symbol), std::to_string(bet_id)}
   );

   uint64_t my_clear_id = 0;

   my_clearing_stats player_bet_clearing_stats(_self, player.value);
   player_bet_clearing_stats.emplace(player, [&](st_mybet_clearing_stats &info) {
      info.id = (uint64_t)player_bet_clearing_stats.available_primary_key();
      info.bet_id = bet_id;
      info.amount = bet_amount;
      info.roll_under = roll_under;
      info.random_roll = 0;
      info.bet_time = time_point_sec(now());
      my_clear_id = info.id;
   });

   bet_stats.emplace(player, [&](st_bet_stats &info) {
      info.id = bet_id;
      info.player = player;
      info.amount = bet_amount;
      info.roll_under = roll_under;
      info.bet_time = time_point_sec(now());
      info.my_clear_id = my_clear_id;
   });
}

void pocker::resolvebet(const uint64_t bet_id)
{
   require_auth(_self);
   potato::check(_code == _self, "the action only support owner");
   auto it = bet_stats.find(bet_id);
   potato::check(it != bet_stats.end(), "bet id is not fonud.");
   // potato::check(it->status == 0, "the bet already check clearing.");

   auto s = read_transaction(nullptr, 0);
   char *tx = (char *)malloc(s);
   read_transaction(tx, s);
   auto tx_hash = potato::sha256(tx, s).data();

   uint64_t random_roll = 0;
   for (uint8_t i = 0; i < 8; ++i)
   {
      random_roll += (uint64_t)tx_hash[i];
   }
   random_roll = (random_roll % (uint64_t)95) + 5;

   name player = it->player;
   uint64_t house_edge = HOUSEEDGE_REF_times10000;
   uint64_t your_bet_amount = it->amount.amount;
   uint64_t roll_under = it->roll_under;

   // uint64_t payout = 0;
   // uint64_t ref_reward = 0;
   // if (random_roll < roll_under)
   // {
   //    payout = (your_bet_amount * get_payout_mult_times10000(roll_under, house_edge)) / 10000;
   // }
   // ref_reward = your_bet_amount * REFERRER_REWARD_times10000 / 10000;   // uint64_t payout = 0;
   asset zero_poc(0, token_symbol);
   asset payout(0, token_symbol);
   asset ref_reward(0, token_symbol);
   if (random_roll < roll_under)
   {
      payout = (it->amount * get_payout_mult_times10000(roll_under, house_edge)) / 10000;
   }
   ref_reward = (it->amount * REFERRER_REWARD_times10000) / 10000;
   // print("payout:");payout.print();print('\n');
   // print("reward:");ref_reward.print();print('\n');

   transaction transfer;
   if (payout > zero_poc)
   {
      // INLINE_ACTION_SENDER(potato::token, transfer)(
      //    token_account, { {_self, active_permission} },
      //    { _self, player, asset(payout, token_symbol), "unallocated inflation" }
      // );
      transfer.actions.emplace_back(
          potato::permission_level{_self, active_permission},
          token_account, transfer_action,
          std::make_tuple(
              _self,
              player,
              // asset(payout, token_symbol),
              payout,
              std::string("Bet id: ") + std::to_string(it->id) + std::string(" -- Winner ") + std::to_string(roll_under) + std::string(">") + std::to_string(random_roll)
            )
         );
   }

   if (ref_reward > zero_poc)
   {
      // INLINE_ACTION_SENDER(potato::token, transfer)(
      //    token_account, { {_self, active_permission} },
      //    { _self, pocker_edge, asset(ref_reward, token_symbol), "unallocated inflation" }
      // );
      transfer.actions.emplace_back(
          potato::permission_level{_self, active_permission},
          token_account,
          transfer_action,
          std::make_tuple(
              _self,
              pocker_edge,
              // asset(ref_reward, token_symbol),
              ref_reward,
              std::string("Bet id: ") + std::to_string(it->id) + std::string(" -- ref") + std::to_string(roll_under) + std::string(" ") + std::to_string(random_roll)));
   }

   // transfer.delay_sec = 5;
   transfer.send(0, _self, false);

   bet_stats.erase(it);
   bet_clearing_stats.emplace(_self, [&](st_bet_clearing_stats &info) {
      info.id = (uint64_t)bet_clearing_stats.available_primary_key();
      info.bet_id = bet_id;
      info.player = player;
      info.amount = it->amount;
      info.roll_under = it->roll_under;
      info.random_roll = random_roll;
      info.bet_time = it->bet_time;
   });
   // print("my_clear_id:");print(it->my_clear_id);print('\n');
   // print("player:");printn(&player.value);print('\n');
   // print("player:");print(player.value);print('\n');
   
   my_clearing_stats player_bet_clearing_stats(_self, player.value);
   auto it2 = player_bet_clearing_stats.find(it->my_clear_id);
   if (it2 != player_bet_clearing_stats.end()) {
      player_bet_clearing_stats.modify(it2, player, [&](st_mybet_clearing_stats &info) {
         info.random_roll = random_roll;
         // print("random_roll:");print(random_roll);print('\n');
      });
   }
}

void pocker::refundbet(const name player, const uint64_t bet_id)
{
   require_auth(player);
   auto it = bet_stats.find(bet_id);
   potato::check(it != bet_stats.end(), "game round is not fonud.");
   // potato::check(it->status == 0, "the bet already check clearing.");
   const time_point_sec bet_time = it->bet_time;
   potato::check(time_point_sec(now() - 5 * 60) > bet_time, "wait 10 minutes");
   potato::check(it->player == player, "you are not beter");

   INLINE_ACTION_SENDER(potato::token, transfer)
   (
      token_account, {{_self, active_permission}},
      {_self, player, it->amount, std::string(" Bet id: ") + std::to_string(bet_id) + std::string(" -- REFUND. Sorry for the inconvenience.")}
   );
   bet_stats.erase(it);
}

uint64_t pocker::get_token_balance(const name account, const symbol_code &token_type) const
{
   auto balance = potato::token::get_balance("pc.token"_n, account, token_type);
   return (uint64_t)balance.amount;
}

uint64_t pocker::get_payout_mult_times10000(const uint32_t roll_under, const uint64_t house_edge_times_10000) const
{
   return ((10000 - house_edge_times_10000) * 100) / (roll_under - 1);
}

uint64_t pocker::get_max_win() const
{
   const uint64_t poc_balance = get_token_balance(_self, token_symbol.code());
   return (poc_balance) / 10;
}

void pocker::test()
{
   require_auth(_self);
   auto itr = bet_clearing_stats.begin();
   while (itr != bet_clearing_stats.end())
   {
      itr = bet_clearing_stats.erase(itr);
   }
}

void pocker::test2(const name player)
{
   require_auth(_self);
   my_clearing_stats player_bet_clearing_stats(_self, player.value);
   auto it2 = player_bet_clearing_stats.begin();
   while (it2 != player_bet_clearing_stats.end())
   {
      it2 = player_bet_clearing_stats.erase(it2);
   }
}

} // namespace potato

POTATO_DISPATCH(potato::pocker, (bet)(resolvebet)(refundbet)(test)(test2))
