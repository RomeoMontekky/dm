cmake_minimum_required(VERSION 3.0)

include("../common.cmake")

set(BINARY_NAME "engine")

set(CPP_FILES 
   "implementation/common/bracket_utils.cpp"
   "implementation/common/exception.cpp"
   "implementation/common/function_utils.cpp"
   "implementation/common/literals.cpp"
   "implementation/common/named_entity.cpp"
   "implementation/common/operations.cpp"
   "implementation/common/qualifier_utils.cpp"
   "implementation/common/string_utils.cpp"

   "implementation/expressions/expression_base.cpp"
   "implementation/expressions/expression_calculator.cpp"
   "implementation/expressions/expression_comparator.cpp"
   "implementation/expressions/expression_evaluator.cpp"
   "implementation/expressions/expression_literal.cpp"
   "implementation/expressions/expression_normalizer.cpp"
   "implementation/expressions/expression_operation.cpp"
   "implementation/expressions/expression_param_ref.cpp"
   "implementation/expressions/expression_simplifier.cpp"
   "implementation/expressions/expression_visitor.cpp"

   "implementation/functions/function_base.cpp"
   "implementation/functions/function_manager.cpp"
   "implementation/functions/function_output.cpp"

   "implementation/functions/impl/function_copy.cpp"
   "implementation/functions/impl/function_display.cpp"
   "implementation/functions/impl/function_display_all.cpp"
   "implementation/functions/impl/function_eval.cpp"
   "implementation/functions/impl/function_print.cpp"
   "implementation/functions/impl/function_remove.cpp"
   "implementation/functions/impl/function_remove_all.cpp"
   "implementation/functions/impl/function_table.cpp"
  
   "implementation/variables/variable.cpp"
   "implementation/variables/variable_declaration.cpp"
   "implementation/variables/variable_manager.cpp"

   "implementation/engine.cpp"
   "implementation/expression_parser.cpp"
   "implementation/function_caller.cpp"
)

set(HEADER_FILES
   "implementation/common/bracket_utils.h"
   "implementation/common/exception.h"
   "implementation/common/function_utils.h"
   "implementation/common/literals.h"
   "implementation/common/local_array.h"
   "implementation/common/named_entity.h"
   "implementation/common/noncopyable.h"
   "implementation/common/operations.h"
   "implementation/common/qualifier_utils.h"
   "implementation/common/string_utils.h"

   "implementation/expressions/expression_base.h"
   "implementation/expressions/expression_calculator.h"
   "implementation/expressions/expression_comparator.h"
   "implementation/expressions/expression_evaluator.h"
   "implementation/expressions/expression_literal.h"
   "implementation/expressions/expression_normalizer.h"
   "implementation/expressions/expression_operation.h"
   "implementation/expressions/expression_param_ref.h"
   "implementation/expressions/expression_simplifier.h"
   "implementation/expressions/expression_visitor.h"
   "implementation/expressions/expressions.h"

   "implementation/functions/function_base.h"
   "implementation/functions/function_manager.h"
   "implementation/functions/function_output.h"
   "implementation/functions/function_registrator.h"

   "implementation/variables/variable.h"
   "implementation/variables/variable_declaration.h"
   "implementation/variables/variable_manager.h"

   "implementation/engine.h"
   "implementation/expression_parser.h"
   "implementation/function_caller.h"
)

set(PUBLIC_HEADER_FILES
   "engine/iengine.h"
   "engine/istringable.h"
   "engine/iexception.h"
   "engine/impexp.h"
)

add_definitions("-DENGINE_EXPORT")

include_directories(".")

add_library(${BINARY_NAME} SHARED 
   ${CPP_FILES} ${HEADER_FILES} ${PUBLIC_HEADER_FILES})

# From "common.cmake"
set_options_and_post_build_steps()
