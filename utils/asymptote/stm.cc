/*****
 * stm.cc
 * Andy Hammerlindl 2002/8/30
 *
 * Statements are everything in the language that do something on their
 * own.  Statements are different from declarations in that statements
 * do not modify the environment.  Translation of a statement puts the
 * stack code to run it into the instruction stream.
 *****/

#include <fstream>
#include "errormsg.h"
#include "settings.h"
#include "coenv.h"
#include "exp.h"
#include "stm.h"

#include "symbol.h"
#include "opsymbols.h"

namespace absyntax {

using namespace trans;
using namespace types;

void stm::prettyprint(ostream &out, Int indent)
{
  prettyname(out,"stm",indent);
}


void emptyStm::prettyprint(ostream &out, Int indent)
{
  prettyname(out,"emptyStm",indent);
}


void blockStm::prettyprint(ostream &out, Int indent)
{
  prettyname(out,"blockStm",indent);

  base->prettyprint(out, indent+1);
}


void expStm::prettyprint(ostream &out, Int indent)
{
  prettyname(out,"expStm",indent);

  body->prettyprint(out, indent+1);
}

void baseExpTrans(coenv &e, exp *expr)
{
  types::ty_kind kind = expr->trans(e)->kind;
  if (kind != types::ty_void)
    // Remove any value it puts on the stack.
    e.c.encodePop();
}

void expStm::trans(coenv &e) {
  baseExpTrans(e, body);
}

// For an object such as currentpicture, write 'picture currentpicture' to
// give some information.  Only do this when the object has a name.
void tryToWriteTypeOfExp(types::ty *t, exp *body)
{
  symbol name=body->getName();
  if (!name)
    return;

  overloaded *set = dynamic_cast<overloaded *>(t);
  if (set)
    for(ty_vector::iterator ot=set->sub.begin(); ot!=set->sub.end(); ++ot)
      tryToWriteTypeOfExp(*ot, body);
  else {
    cout << "<";
    t->printVar(cout, name);
    cout << ">" << endl;
  }
}
  
// From dec.cc:
varEntry *makeVarEntry(position pos, coenv &e, record *r, types::ty *t);

void storeExp(coenv &e, types::ty *t, exp *expr) {
  assert(t->kind != ty_error);
  assert(t->kind != ty_void);
  assert(t->kind != ty_overloaded);

  expr->transAsType(e, t);

  // Store the value in a new variable of the proper type.
  varEntry *v = makeVarEntry(expr->getPos(), e, 0, t);
  e.e.addVar(symbol::trans("operator answer"), v);
  v->getLocation()->encode(WRITE, expr->getPos(), e.c);
  e.c.encodePop();
}

void storeAndWriteExp(coenv &e, types::ty *t, exp *expr) {
  storeExp(e, t, expr);

  position pos=expr->getPos();
  baseExpTrans(e, new callExp(pos, new nameExp(pos, "write"),
                                   new nameExp(pos, "operator answer")));
}

void tryToWriteExp(coenv &e, exp *expr)
{
  position pos=expr->getPos();
  types::ty *t=expr->cgetType(e);

  // If the original expression is bad, just print the errors.
  // If it is a function which returns void, just call the function.
  if (t->kind == ty_error || t->kind == ty_void) {
    baseExpTrans(e, expr);
    return;
  }

  exp *callee=new nameExp(pos, symbol::trans("write"));
  exp *call=new callExp(pos, callee, expr);

  types::ty *ct=call->getType(e);
  if (ct->kind == ty_error || ct->kind == ty_overloaded) {
    if (t->kind == ty_overloaded) {
      // Translate the expr in order to print the ambiguity error first.
      expr->trans(e);
      em.sync();
      assert(em.errors());
      
      // Then, write out all of the types.
      tryToWriteTypeOfExp(t, expr);
    }
    else {
      // Write the type of the expression and, since it is unique, assign it to
      // 'operator answer' even though its value isn't printed.
      tryToWriteTypeOfExp(t, expr);
      storeExp(e, t, expr);
    }
  }
  else if (t->kind == ty_overloaded) {
    // If the exp is overloaded, but the act of writing makes it
    // unambiguous, add a suffix to the output to warn the user of this.
    exp *suffix=new nameExp(pos,
                            symbol::trans("overloadedMessage"));
    exp *callWithSuffix=new callExp(pos,
                                    callee, expr, suffix);

    if (callWithSuffix->getType(e)->kind != ty_error)
      baseExpTrans(e, callWithSuffix);
    else
      baseExpTrans(e, call);
  }
  else {
    // Interactive writing can proceed normally.
    storeAndWriteExp(e, t, expr);
  }
}

void expStm::interactiveTrans(coenv &e)
{
  // First check if it is the kind of expression that should be written.
  if (body->writtenToPrompt() && 
      settings::getSetting<bool>("interactiveWrite"))
    tryToWriteExp(e, body);
  else
    baseExpTrans(e, body);
}


void ifStm::prettyprint(ostream &out, Int indent)
{
  prettyname(out,"ifStm",indent);

  test->prettyprint(out, indent+1);
  onTrue->prettyprint(out, indent+1);
  if (onFalse)
    onFalse->prettyprint(out, indent+1);
}

void ifStm::trans(coenv &e)
{
  Int elseLabel = e.c.fwdLabel();
  Int end = e.c.fwdLabel();

#ifdef TRANSJUMP
  test->transConditionalJump(e, false, elseLabel);
#else
  test->transToType(e, types::primBoolean());
  e.c.useLabel(inst::njmp,elseLabel);
#endif

  onTrue->markTrans(e);
  
  if (onFalse) {
    // Encode the jump around the 'else' clause at the end of the 'if' clause
    e.c.useLabel(inst::jmp,end);

    e.c.defLabel(elseLabel);
    onFalse->markTrans(e);
  } else {
    e.c.defLabel(elseLabel);
  }


  e.c.defLabel(end);
}


void transLoopBody(coenv &e, stm *body) {
  e.c.encodePushFrame();
  body->markTrans(e);
  e.c.encodePopFrame();
}

void whileStm::prettyprint(ostream &out, Int indent)
{
  prettyname(out,"whileStm",indent);

  test->prettyprint(out, indent+1);
  body->prettyprint(out, indent+1);
}

void whileStm::trans(coenv &e)
{
  Int end = e.c.fwdLabel();
  e.c.pushBreak(end);

  Int start = e.c.defLabel();
  e.c.pushContinue(start);

#ifdef TRANSJUMP
  test->transConditionalJump(e, false, end);
#else
  test->transToType(e, types::primBoolean());
  e.c.useLabel(inst::njmp,end);
#endif

  transLoopBody(e,body);

  e.c.useLabel(inst::jmp,start);
  e.c.defLabel(end);

  e.c.popBreak();
  e.c.popContinue();
}


void doStm::prettyprint(ostream &out, Int indent)
{
  prettyname(out,"doStm",indent);

  body->prettyprint(out, indent+1);
  test->prettyprint(out, indent+1);
}

void doStm::trans(coenv &e)
{
  Int testLabel = e.c.fwdLabel();
  e.c.pushContinue(testLabel);
  Int end = e.c.fwdLabel();
  e.c.pushBreak(end);
 
  Int start = e.c.defLabel();

  transLoopBody(e,body);  
  
  e.c.defLabel(testLabel);

#ifdef TRANSJUMP
  test->transConditionalJump(e, true, start);
#else
  test->transToType(e, types::primBoolean());
  e.c.useLabel(inst::cjmp,start);
#endif

  e.c.defLabel(end);

  e.c.popBreak();
  e.c.popContinue();
}


void forStm::prettyprint(ostream &out, Int indent)
{
  prettyname(out,"forStm",indent);

  if (init) init->prettyprint(out, indent+1);
  if (test) test->prettyprint(out, indent+1);
  if (update) update->prettyprint(out, indent+1);
  body->prettyprint(out, indent+1);
}

void forStm::trans(coenv &e)
{
  // Any vardec in the initializer needs its own scope.
  e.e.beginScope();
  if (init)
    init->markTrans(e);

  Int ctarget = e.c.fwdLabel();
  e.c.pushContinue(ctarget);
  Int end = e.c.fwdLabel();
  e.c.pushBreak(end);

  Int start = e.c.defLabel();
  if(test) {
#ifdef TRANSJUMP
    test->transConditionalJump(e, false, end);
#else
    test->transToType(e, types::primBoolean());
    e.c.useLabel(inst::njmp,end);
#endif
  }

  transLoopBody(e,body);

  e.c.defLabel(ctarget);
  
  if (update)
    update->markTrans(e);
  e.c.useLabel(inst::jmp,start);

  e.c.defLabel(end);

  e.e.endScope();
  e.c.popBreak();
  e.c.popContinue();
}

void extendedForStm::prettyprint(ostream &out, Int indent)
{
  prettyindent(out, indent);
  out << "extendedForStm: '" << var << "'\n";

  start->prettyprint(out, indent+1);
  set->prettyprint(out, indent+1);
  body->prettyprint(out, indent+1);
}

void extendedForStm::trans(coenv &e) {
  // Translate into the syntax:
  //
  // start[] a = set;
  // for (int i=0; i < a.length; ++i) {
  //   start var=a[i];
  //   body
  // }

  position pos=getPos();

  // Use gensyms for the variable names so as not to pollute the namespace.
  symbol a=symbol::gensym("a");
  symbol i=symbol::gensym("i");

  // Get the start type.  Handle type inference as a special case.
  types::ty *t = start->trans(e, true);
  if (t->kind == types::ty_inferred) {
    // var a=set;
    tyEntryTy tet(pos, primInferred());
    decid dec1(pos, new decidstart(pos, a), set);
    vardec(pos, &tet, &dec1).trans(e);
  }
  else {
    // start[] a=set;
    arrayTy at(pos, start, new dimensions(pos));
    decid dec1(pos, new decidstart(pos, a), set);
    vardec(pos, &at, &dec1).trans(e);
  }

  // { start var=a[i]; body }
  block b(pos);
  decid dec2(pos,
             new decidstart(pos, var), 
             new subscriptExp(pos, new nameExp(pos, a),
                              new nameExp(pos, i)));
  b.add(new vardec(pos, start, &dec2));
  b.add(body);

  // If there are errors already, just report errors in the body and don't try
  // to translate the for loop.
  if (em.errors()) {
    b.trans(e);
    return;
  }

  // for (int i=0; i < a.length; ++i)
  //   <block>
  forStm(pos,
         new vardec(pos, new tyEntryTy(pos, primInt()),
                         new decid(pos,
                                   new decidstart(pos, i),
                                   new intExp(pos, 0))),
         new binaryExp(pos,
                       new nameExp(pos, i),
                       SYM_LT,
                       new nameExp(pos,
                                   new qualifiedName(pos,
                                                     new simpleName(pos, a),
                                                     symbol::trans("length")))),
         new expStm(pos, new prefixExp(pos, new nameExp(pos, i), SYM_PLUS)),
         new blockStm(pos, &b)).trans(e);
}
                              

void breakStm::prettyprint(ostream &out, Int indent)
{
  prettyname(out,"breakStm",indent);
}

void breakStm::trans(coenv &e)
{
  // Loop bodies have their own frame to declare variables for each iteration.
  // Pop out of this frame when jumping out of the loop body.
  e.c.encode(inst::popframe);

  if (!e.c.encodeBreak()) {
    em.error(getPos());
    em << "break statement outside of a loop";
  }
}


void continueStm::prettyprint(ostream &out, Int indent)
{
  prettyname(out,"continueStm",indent);
}

void continueStm::trans(coenv &e)
{
  // Loop bodies have their own frame to declare variables for each iteration.
  // Pop out of this frame when jumping out of the loop body.
  e.c.encode(inst::popframe);

  if (!e.c.encodeContinue()) {
    em.error(getPos()); 
    em << "continue statement outside of a loop";
  }
}


void returnStm::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "returnStm",indent);

  if (value)
    value->prettyprint(out, indent+1);
}

void returnStm::trans(coenv &e)
{
  types::ty *t = e.c.getReturnType();

  if (t->kind == ty_void) {
    if (value) {
      em.error(getPos());
      em << "function cannot return a value";
    }
    if (e.c.isRecord())
      e.c.encode(inst::pushclosure);
  }
  else {
    if (value) {
      value->transToType(e, t);
    }
    else {
      em.error(getPos());
      em << "function must return a value";
    }
  }

  // NOTE: Currently, a return statement in a module definition will end
  // the initializer.  Should this be allowed?
  e.c.encode(inst::ret);
}


void stmExpList::prettyprint(ostream &out, Int indent)
{
  prettyname(out, "stmExpList",indent);

  for (mem::list<stm *>::iterator p = stms.begin(); p != stms.end(); ++p)
    (*p)->prettyprint(out, indent+1);
}

void stmExpList::trans(coenv &e)
{
  for (mem::list<stm *>::iterator p = stms.begin(); p != stms.end(); ++p)
    (*p)->markTrans(e);
}


} // namespace absyntax
