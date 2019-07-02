/**
 *  @file
 *  @copyright defined in potato/LICENSE
 */

#include <poker/poker.hpp>
#include <pc.token/pc.token.hpp>
#include <potatolib/print.hpp>
#include <potatolib/transaction.hpp>

namespace potato
{

poker::poker(name receiver, name code, datastream<const char *> ds)
    : contract(receiver, code, ds)
      , bet_stats(_self, _self.value)
      , bet_clearing_stats(_self, _self.value)
{
}

void poker::bet(const name player, const asset amount, const std::vector<uint8_t>& rollnum)
{
   require_auth(player);
   potato_assert(amount.symbol == token_symbol, "asset symbol error");
   potato_assert(amount.amount > 0, "Less than minimum limit amount");
   // potato_assert(amount.amount <= get_max_win(), "Exceed maximum limit amount");
   potato_assert(0 < rollnum.size() && rollnum.size() < 13, "Card count must be > 0, and < 13.");

   asset your_win_amount = amount * (odds[rollnum.size() - 1] * 10000);
   your_win_amount /= 10000;
   your_win_amount -= amount;
   potato_assert(your_win_amount <= get_max_win(), "Bet less than max");

   auto s = read_transaction(nullptr, 0);
   char *tx = (char *)malloc(s);
   read_transaction(tx, s);
   auto tx_hash = potato::sha256(tx, s).data();

   const uint64_t bet_id = 
      ((uint64_t)tx_hash[0] << 56) + ((uint64_t)tx_hash[1] << 48) + ((uint64_t)tx_hash[2] << 40) + ((uint64_t)tx_hash[3] << 32) 
      + ((uint64_t)tx_hash[4] << 24) + ((uint64_t)tx_hash[5] << 16) + ((uint64_t)tx_hash[6] << 8) + (uint64_t)tx_hash[7];

   INLINE_ACTION_SENDER(potato::token, transfer)
   (
      token_account, {{player, active_permission}},
      {player, _self, asset(amount.amount, token_symbol), std::to_string(bet_id)}
   );

   uint64_t my_clear_id = 0;

   my_clearing_stats player_bet_clearing_stats(_self, player.value);
   player_bet_clearing_stats.emplace(player, [&](st_mybet_clearing_stats &info) {
      info.id = (uint64_t)player_bet_clearing_stats.available_primary_key();
      info.bet_id = bet_id;
      info.amount = amount;
      info.card_under = rollnum;
      info.random_card = 0;
      info.bet_time = time_point_sec(now());
      my_clear_id = info.id;
   });

   bet_stats.emplace(player, [&](st_bet_stats &info) {
      info.id = bet_id;
      info.player = player;
      info.amount = amount;
      info.card_under = rollnum;
      info.bet_time = time_point_sec(now());
      info.my_clear_id = my_clear_id;
   });
}

void poker::resolvebet(const uint64_t betid)
{
   // potato::check(false, "test");
   require_auth(_self);
   potato::check(_code == _self, "the action only support owner");
   auto it = bet_stats.find(betid);
   potato::check(it != bet_stats.end(), "bet id is not fonud.");
   // potato::check(it->status == 0, "the bet already check clearing.");

   auto s = read_transaction(nullptr, 0);
   char *tx = (char *)malloc(s);
   read_transaction(tx, s);
   auto tx_hash = potato::sha256(tx, s).data();

   uint64_t random_card = 0;
   for (uint8_t i = 0; i < 8; ++i)
   {
      random_card += (uint64_t)tx_hash[i] % (uint64_t)13;
   }
   random_card = (random_card % (uint64_t)13) + 1;
   // print("random_card:");print(random_card);print('\n');

   name player = it->player;
   bool win = false;
   string strcard;
   for (auto card : it->card_under)
   {
      win |= (card == random_card);
      strcard += std::to_string(card) + ",";
   }
   strcard.pop_back();

   static const asset zero_poc(0, token_symbol);
   asset payout(0, token_symbol);
   asset ref_reward(0, token_symbol);
   if (win)
   {
      payout = it->amount;
      payout *= (odds[it->card_under.size() - 1] * 10000);
      payout /= 10000;
      ref_reward = payout / 100;
      payout -= ref_reward;
   }
   else
   {
      ref_reward = it->amount / 100;
   }
   // print("payout:");payout.print();//print('\n');
   // print("reward:");ref_reward.print();//print('\n');
   // print("strcard:");print(strcard.c_str());//print('\n');

   transaction transfer;
   if (payout > zero_poc)
   {
      // INLINE_ACTION_SENDER(potato::token, transfer)(
      //    token_account, { {_self, active_permission} },
      //    { _self, player, payout, "unallocated inflation" }
      // );
      transfer.actions.emplace_back(
          potato::permission_level{_self, active_permission},
          token_account, transfer_action,
          std::make_tuple(
              _self,
              player,
              payout,
              std::string("Bet id: ") + std::to_string(it->id) + std::string(" -- ") + std::to_string(random_card) + std::string(" in ") + strcard
            )
         );
   }

   if (ref_reward > zero_poc)
   {
      // INLINE_ACTION_SENDER(potato::token, transfer)(
      //    token_account, { {_self, active_permission} },
      //    { _self, poker_edge, ref_reward, "unallocated inflation" }
      // );
      transfer.actions.emplace_back(
          potato::permission_level{_self, active_permission},
          token_account,
          transfer_action,
          std::make_tuple(
              _self,
              poker_edge,
              ref_reward,
              std::string("Bet id: ") + std::to_string(it->id) + std::string(" -- ") + std::to_string(random_card)));
   }

   // transfer.delay_sec = 5;
   transfer.send(0, _self, false);

   bet_stats.erase(it);
   bet_clearing_stats.emplace(_self, [&](st_bet_clearing_stats &info) {
      info.id = (uint64_t)bet_clearing_stats.available_primary_key();
      info.bet_id = betid;
      info.player = player;
      info.amount = it->amount;
      info.card_under = it->card_under;
      info.random_card = random_card;
      info.bet_time = it->bet_time;
   });
   // print("my_clear_id:");print(it->my_clear_id);print('\n');
   // print("player:");printn(&player.value);print('\n');
   // print("player:");print(player.value);print('\n');
   
   my_clearing_stats player_bet_clearing_stats(_self, player.value);
   auto it2 = player_bet_clearing_stats.find(it->my_clear_id);
   if (it2 != player_bet_clearing_stats.end()) {
      player_bet_clearing_stats.modify(it2, player, [&](st_mybet_clearing_stats &info) {
         info.random_card = random_card;
         // print("random_roll:");print(random_roll);print('\n');
      });
   }
}

void poker::refundbet(const name player, const uint64_t betid)
{
   require_auth(player);
   auto it = bet_stats.find(betid);
   potato::check(it != bet_stats.end(), "game round is not fonud.");
   // potato::check(it->status == 0, "the bet already check clearing.");
   const time_point_sec bet_time = it->bet_time;
   potato::check(time_point_sec(now() - 5 * 60) > bet_time, "wait 10 minutes");
   potato::check(it->player == player, "you are not beter");

   INLINE_ACTION_SENDER(potato::token, transfer)
   (
      token_account, {{_self, active_permission}},
      {_self, player, it->amount, std::string(" Bet id: ") + std::to_string(betid) + std::string(" -- REFUND. Sorry for the inconvenience.")}
   );
   bet_stats.erase(it);
}

asset poker::get_token_balance(const name account, const symbol_code &token_type) const
{
   auto balance = potato::token::get_balance("pc.token"_n, account, token_type);
   return balance;
}

asset poker::get_max_win() const
{
   const asset poc_balance = get_token_balance(_self, token_symbol.code());
   return (poc_balance) / 10;
}

// void poker::test()
// { 
//    require_auth(_self);
//    auto itr = bet_clearing_stats.begin();
//    while (itr != bet_clearing_stats.end())
//    {
//       itr = bet_clearing_stats.erase(itr);
//    }
// }

// void poker::test2(const name player)
// {
//    require_auth(_self);
//    my_clearing_stats player_bet_clearing_stats(_self, player.value);
//    auto it2 = player_bet_clearing_stats.begin();
//    while (it2 != player_bet_clearing_stats.end())
//    {
//       it2 = player_bet_clearing_stats.erase(it2);
//    }
// }

} // namespace potato

POTATO_DISPATCH(potato::poker, (bet)(resolvebet)(refundbet)/*(test)(test2) */)
