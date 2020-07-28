// clang-format off
// Hardcode
void CreateModel_quant8_no_axis(Model *model) {
  OperandType type0(Type::TENSOR_QUANT8_ASYMM, {2, 3, 4, 5}, 1.0f, 0);
  OperandType type1(Type::TENSOR_INT32, {4});
  OperandType type2(Type::TENSOR_QUANT8_ASYMM, {5, 4, 3, 2}, 1.0f, 0);
  // Phase 1, operands
  auto input = model->addOperand(&type0);
  auto perms = model->addOperand(&type1);
  auto output = model->addOperand(&type2);
  // Phase 2, operations
  model->setOperandValue(perms, nullptr, 0);
  model->addOperation(ANEURALNETWORKS_TRANSPOSE, {input, perms}, {output});
  // Phase 3, inputs and outputs
  model->identifyInputsAndOutputs(
    {input},
    {output});
  assert(model->isValid());
}

inline bool is_ignored(int i) {
  static std::set<int> ignore = {};
  return ignore.find(i) != ignore.end();
}

void CreateModel_quant8_no_axis_simple(Model *model) {
  OperandType type0(Type::TENSOR_QUANT8_ASYMM, {2, 3}, 1.0f, 0);
  OperandType type1(Type::TENSOR_INT32, {4});
  OperandType type2(Type::TENSOR_QUANT8_ASYMM, {3, 2}, 1.0f, 0);
  // Phase 1, operands
  auto input = model->addOperand(&type0);
  auto perms = model->addOperand(&type1);
  auto output = model->addOperand(&type2);
  // Phase 2, operations
  model->setOperandValue(perms, nullptr, 0);
  model->addOperation(ANEURALNETWORKS_TRANSPOSE, {input, perms}, {output});
  // Phase 3, inputs and outputs
  model->identifyInputsAndOutputs(
    {input},
    {output});
  assert(model->isValid());
}
