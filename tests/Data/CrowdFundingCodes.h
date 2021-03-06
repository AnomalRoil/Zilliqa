/*
 * Copyright (c) 2018 Zilliqa
 * This source code is being disclosed to you solely for the purpose of your
 * participation in testing Zilliqa. You may view, compile and run the code for
 * that purpose and pursuant to the protocols and algorithms that are programmed
 * into, and intended by, the code. You may not do anything else with the code
 * without express permission from Zilliqa Research Pte. Ltd., including
 * modifying or publishing the code (or any part of it), and developing or
 * forming another public or private blockchain network. This source code is
 * provided 'as is' and no warranties are given as to title or non-infringement,
 * merchantability or fitness for purpose and, to the extent permitted by law,
 * all liability for your use of the code is disclaimed. Some programs in this
 * code are governed by the GNU General Public License v3.0 (available at
 * https://www.gnu.org/licenses/gpl-3.0.en.html) ('GPLv3'). The programs that
 * are governed by GPLv3.0 are those programs that are located in the folders
 * src/depends and tests/depends and which include a reference to GPLv3 in their
 * program files.
 */
#include <string>

std::string cfCodeStr = R"((***************************************************)
(*               Associated library                *)
(***************************************************)
library Crowdfunding

let andb = 
  fun (b : Bool) =>
  fun (c : Bool) =>
    match b with 
    | False => False
    | True  =>
      match c with 
      | False => False
      | True  => True
      end
    end

let orb = 
  fun (b : Bool) => fun (c : Bool) =>
    match b with 
    | True  => True
    | False =>
      match c with 
      | False => False
      | True  => True
      end
    end

let negb = fun (b : Bool) => 
  match b with
  | True => False
  | False => True
  end

let one_msg = 
  fun (msg : Message) => 
    let nil_msg = Nil {Message} in
    Cons {Message} msg nil_msg
    
let check_update = 
  fun (bs : Map Address Uint128) =>
  fun (_sender : Address) =>
  fun (_amount : Uint128) =>
    let c = builtin contains bs _sender in
    match c with 
    | False => 
      let bs1 = builtin put bs _sender _amount in
      Some {Map Address Uint128} bs1 
    | True  => None {Map Address Uint128}
    end

let blk_leq =
  fun (blk1 : BNum) =>
  fun (blk2 : BNum) =>
    let bc1 = builtin blt blk1 blk2 in 
    let bc2 = builtin eq blk1 blk2 in 
    orb bc1 bc2

let accepted_code = Int32 1
let missed_deadline_code = Int32 2
let already_backed_code  = Int32 3
let not_owner_code  = Int32 4
let too_early_code  = Int32 5
let got_funds_code  = Int32 6
let cannot_get_funds  = Int32 7
let cannot_reclaim_code = Int32 8
let reclaimed_code = Int32 9
  
(***************************************************)
(*             The contract definition             *)
(***************************************************)
contract Crowdfunding

(*  Parameters *)
(owner     : Address,
 max_block : BNum,
 goal      : Uint128)

(* Mutable fields *)
field backers : Map Address Uint128 = Emp Address Uint128
field funded : Bool = False

transition Donate ()
  blk <- & BLOCKNUMBER;
  in_time = blk_leq blk max_block;
  match in_time with 
  | True  => 
    bs  <- backers;
    res = check_update bs _sender _amount;
    match res with
    | None => 
      msg  = {_tag : ""; _recipient : _sender; _amount : Uint128 0; 
              code : already_backed_code};
      msgs = one_msg msg;
      send msgs
    | Some bs1 =>
      backers := bs1; 
      accept; 
      msg  = {_tag : ""; _recipient : _sender; _amount : Uint128 0; 
              code : accepted_code};
      msgs = one_msg msg;
      send msgs     
    end  
  | False => 
    msg  = {_tag : ""; _recipient : _sender; _amount : Uint128 0; 
            code : missed_deadline_code};
    msgs = one_msg msg;
    send msgs
  end 
end

transition GetFunds ()
  is_owner = builtin eq owner _sender;
  match is_owner with
  | False => 
    msg  = {_tag : ""; _recipient : _sender; _amount : Uint128 0;
            code : not_owner_code};
    msgs = one_msg msg;
    send msgs
  | True => 
    blk <- & BLOCKNUMBER;
    in_time = blk_leq blk max_block;
    c1 = negb in_time;
    bal <- balance;
    c2 = builtin lt bal goal;
    c3 = negb c2;
    c4 = andb c1 c3;
    match c4 with 
    | False =>  
      msg  = {_tag : ""; _recipient : _sender; _amount : Uint128 0;
              code : cannot_get_funds};
      msgs = one_msg msg;
      send msgs
    | True => 
      tt = True;
      funded := tt;
      msg  = {_tag : Main; _recipient : owner; _amount : bal; 
              code : got_funds_code};
      msgs = one_msg msg;
      send msgs
    end
  end   
end

(* transition ClaimBack *)
transition ClaimBack ()
  blk <- & BLOCKNUMBER;
  after_deadline = builtin blt max_block blk;
  match after_deadline with
  | False =>
    msg  = {_tag : ""; _recipient : _sender; _amount : Uint128 0;
            code : too_early_code};
    msgs = one_msg msg;
    send msgs
  | True =>
    bs <- backers;
    bal <- _balance;
    (* Goal has not been reached *)
    f <- funded;
    c1 = builtin lt bal goal;
    c2 = builtin contains bs _sender;
    c3 = negb f;
    c4 = andb c1 c2;
    c5 = andb c3 c4;
    match c5 with
    | False =>
      msg  = {_tag : ""; _recipient : _sender; _amount : Uint128 0;
              code : cannot_reclaim_code};
      msgs = one_msg msg;
      send msgs
    | True =>
      res = builtin get bs _sender;
      match res with
      | None =>
        msg  = {_tag : ""; _recipient : _sender; _amount : Uint128 0;
                code : cannot_reclaim_code};
        msgs = one_msg msg;
        send msgs
      | Some v =>
        bs1 = builtin remove bs _sender;
        backers := bs1;
        msg  = {_tag : Main; _recipient : _sender; _amount : v; 
                code : reclaimed_code};
        msgs = one_msg msg;
        send msgs
      end
    end
  end  
end)";

std::string cfInitStr = R"([
    {
        "vname" : "owner",
        "type" : "Address", 
        "value" : "$ADDR"
    },
    {
        "vname" : "max_block",
        "type" : "BNum" ,
        "value" : "199"
    },
    { 
        "vname" : "goal",
        "type" : "Uint128",
        "value" : "500"
    }
])";

std::string cfDataDonateStr = R"({
    "_tag": "Donate",
    "params": []
}
)";

std::string cfDataClaimBackStr = R"({
    "_tag": "ClaimBack",
    "params": []
})";

std::string cfDataGetFundsStr = R"({
    "_tag": "GetFunds",
    "params": []
})";

std::string cfOutStr = R"({
  "message": {
    "_tag": "Main",
    "_amount": "0",
    "params": [
      {
        "vname": "to",
        "type": "Address",
        "value": "0x12345678901234567890123456789012345678ab"
      },
      { "vname": "code", "type": "Int", "value": "1" }
    ]
  },
  "states": [
    { "vname": "_balance", "type": "Int", "value": "100" },
    {
      "vname": "backers",
      "type": "Map",
      "value": [
        { "keyType": "Address", "valType": "Int" },
        { "key": "0x12345678901234567890123456789012345678ab", "val": "100" }
      ]
    },
    {
      "vname": "funded",
      "type": "ADT",
      "value": { "constructor": "False", "argtypes": [], "arguments": [] }
    }
  ]
})";