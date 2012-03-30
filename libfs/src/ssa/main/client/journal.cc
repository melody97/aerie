#include "ssa/main/client/journal.h"
#include "ssa/main/client/session.h"
#include "ssa/main/common/publisher.h"

namespace ssa {
namespace client {

typedef ::ssa::Publisher::Messages::TransactionBegin         TransactionBeginMessage;
typedef ::ssa::Publisher::Messages::TransactionEnd           TransactionEndMessage;
typedef ::ssa::Publisher::Messages::LogicalOperationHeader   LogicalOperationHeaderMessage;
typedef ::ssa::Publisher::Messages::ContainerOperationHeader ContainerOperationHeaderMessage;


int
Journal::TransactionBegin(int id)
{
	TransactionBeginMessage tx = TransactionBeginMessage(id);
	buffer_.Write(&tx, sizeof(tx));
	return E_SUCCESS;
}


int
Journal::TransactionEnd()
{
	TransactionEndMessage tx = TransactionEndMessage();
	buffer_.Write(&tx, sizeof(tx));
	buffer_.Flush(session_->stsystem()->shbuf());
	return E_SUCCESS;
}


} // namespace client
} // namespace ssa