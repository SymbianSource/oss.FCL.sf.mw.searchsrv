#include "CLucene/StdHeader.h"
#include "SimpleFragmenter.h"

CL_NS_DEF2(search,highlight)
CL_NS_USE(analysis)

SimpleFragmenter::SimpleFragmenter(int32_t fragmentSize)
	: _currentNumFrags(0), _fragmentSize(fragmentSize)
{
}
SimpleFragmenter::~SimpleFragmenter(){
}

void SimpleFragmenter::start(const TCHAR*)
{
	_currentNumFrags=1;
}

bool SimpleFragmenter::isNewFragment(const Token * token)
{
	bool isNewFrag= token->endOffset()>=(_fragmentSize*_currentNumFrags);
	if (isNewFrag) {
		_currentNumFrags++;
	}
	return isNewFrag;
}

int32_t SimpleFragmenter::getFragmentSize() const
{
	return _fragmentSize;
}

void SimpleFragmenter::setFragmentSize(int32_t size)
{
	_fragmentSize = size;
}

CL_NS_END2
