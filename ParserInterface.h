#ifndef __PARSER_INTERFACE_H__
#define __PARSER_INTERFACE_H__

class ParserInterface {
public:
  virtual ~ParserInterface(void) {};
  virtual void Iterate(void) = 0;
};

#endif //__PARSER_INTERFACE_H__

