// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

#include "exprs/binary_predicate.h"

#include <gtest/gtest.h>

#include "common/object_pool.h"
#include "exec/exec_node.h"
#include "exprs/expr.h"
#include "exprs/int_literal.h"
#include "gen_cpp/Exprs_types.h"
#include "runtime/runtime_state.h"
#include "util/debug_util.h"

namespace doris {

class BinaryOpTest : public ::testing::Test {
public:
    ~BinaryOpTest() {}

    virtual void SetUp() {
        _object_pool = new ObjectPool();
        _runtime_state = _object_pool->add(new RuntimeState(""));

        TDescriptorTable ttbl;
        TTupleDescriptor tuple_desc;
        tuple_desc.__set_id(0);
        tuple_desc.__set_byteSize(8);
        tuple_desc.__set_numNullBytes(0);
        ttbl.tupleDescriptors.push_back(tuple_desc);

        TSlotDescriptor slot_desc;
        slot_desc.__set_id(0);
        slot_desc.__set_parent(0);
        slot_desc.__set_slotType(TPrimitiveType::INT);
        slot_desc.__set_columnPos(0);
        slot_desc.__set_byteOffset(4);
        slot_desc.__set_nullIndicatorByte(0);
        slot_desc.__set_nullIndicatorBit(0);
        slot_desc.__set_colName("col1");
        slot_desc.__set_slotIdx(0);
        slot_desc.__set_isMaterialized(true);
        ttbl.slotDescriptors.push_back(slot_desc);

        DescriptorTbl* desc_tbl = nullptr;
        EXPECT_TRUE(DescriptorTbl::create(_object_pool, ttbl, &desc_tbl).ok());
        EXPECT_TRUE(desc_tbl != nullptr);
        _runtime_state->set_desc_tbl(desc_tbl);

        std::vector<TTupleId> row_tuples;
        row_tuples.push_back(0);
        std::vector<bool> nullable_tuples;
        nullable_tuples.push_back(false);
        _row_desc = _object_pool->add(new RowDescriptor(*desc_tbl, row_tuples, nullable_tuples));

        FieldInfo field;
        field.name = "col1";
        field.type = OLAP_FIELD_TYPE_INT;
        field.length = 4;
        field.is_key = true;
        _schema.push_back(field);
    }
    virtual void TearDown() {
        if (_object_pool != nullptr) {
            delete _object_pool;
            _object_pool = nullptr;
        }
    }

    Expr* create_expr() {
        TExpr exprs;
        {
            TExprNode expr_node;
            expr_node.__set_node_type(TExprNodeType::BINARY_PRED);
            TColumnType type;
            type.__set_type(TPrimitiveType::INT);
            expr_node.__set_type(type);
            expr_node.__set_num_children(2);
            expr_node.__isset.opcode = true;
            expr_node.__set_opcode(TExprOpcode::LT_INT_INT);
            expr_node.__isset.vector_opcode = true;
            expr_node.__set_vector_opcode(TExprOpcode::FILTER_LT_INT_INT);
            exprs.nodes.push_back(expr_node);
        }
        {
            TExprNode expr_node;
            expr_node.__set_node_type(TExprNodeType::SLOT_REF);
            TColumnType type;
            type.__set_type(TPrimitiveType::INT);
            expr_node.__set_type(type);
            expr_node.__set_num_children(0);
            expr_node.__isset.slot_ref = true;
            TSlotRef slot_ref;
            slot_ref.__set_slot_id(0);
            slot_ref.__set_tuple_id(0);
            expr_node.__set_slot_ref(slot_ref);
            expr_node.__isset.output_column = true;
            expr_node.__set_output_column(0);
            exprs.nodes.push_back(expr_node);
        }
        {
            TExprNode expr_node;
            expr_node.__set_node_type(TExprNodeType::INT_LITERAL);
            TColumnType type;
            type.__set_type(TPrimitiveType::INT);
            expr_node.__set_type(type);
            expr_node.__set_num_children(0);
            expr_node.__isset.int_literal = true;
            TIntLiteral int_literal;
            int_literal.__set_value(10);
            expr_node.__set_int_literal(int_literal);
            exprs.nodes.push_back(expr_node);
        }
        Expr* root_expr = nullptr;

        if (Expr::create_expr_tree(_object_pool, exprs, &root_expr).ok()) {
            return root_expr;
        } else {
            return nullptr;
        }
    }

public:
    ObjectPool* object_pool() { return _object_pool; }
    RuntimeState* runtime_state() { return _runtime_state; }
    RowDescriptor* row_desc() { return _row_desc; }

private:
    ObjectPool* _object_pool;
    RuntimeState* _runtime_state;
    RowDescriptor* _row_desc;
    std::vector<FieldInfo> _schema;
};

TEST_F(BinaryOpTest, PrepareTest) {
    Expr* expr = create_expr();
    EXPECT_TRUE(expr != nullptr);
    EXPECT_TRUE(expr->prepare(runtime_state(), *row_desc()).ok());
}

} // namespace doris

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
