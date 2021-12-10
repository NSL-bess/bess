#ifndef BESS_MODULES_DISTRIBUTED_FLOW_COUNTER_H_
#define BESS_MODULES_DISTRIBUTED_FLOW_COUNTER_H_

#include <hiredis/hiredis.h>
#include <set>
#include <tuple>

#include "../module.h"
#include "../utils/endian.h"
#include "../utils/flow.h"
#include "../utils/mcslock.h"

using bess::utils::be16_t;
using bess::utils::be32_t;
using bess::utils::Flow;
using bess::utils::FlowHash;

class FlowCounter final: public Module {
public:
  static const Commands cmds;

  CommandResponse Init(const bess::pb::FlowCounterArg &);
  CommandResponse CommandGetSummary(const bess::pb::EmptyArg &);
  CommandResponse CommandStart(const bess::pb::EmptyArg &);
  CommandResponse CommandStop(const bess::pb::EmptyArg &);
  // Reset = Clear + Start;
  CommandResponse CommandReset(const bess::pb::EmptyArg &);
  CommandResponse CommandClear(const bess::pb::EmptyArg &);

  void ProcessBatch(Context *ctx, bess::PacketBatch *batch) override;

private:
  void Start();
  void Stop();
  void Clear();
  void Reset();

  // If true, this module is actively counting packets.
  bool is_active_ = false;
  // The local flow counter cache.
  std::unordered_map<Flow, uint64_t, FlowHash> flow_cache_;

  mcslock lock_;
};

#endif  // BESS_MODULES_DISTRIBUTED_FLOW_COUNTER_H_