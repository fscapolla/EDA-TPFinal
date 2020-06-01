#include "FullNode.h"



FullNode::FullNode()
{
}

FullNode::FullNode(unsigned int ID_, std::string IP_, unsigned int port_)
{
	ID = ID_;
	IP = IP_;
	port = port_;
	client = new NodeClient(IP, port);
}


FullNode::~FullNode()
{
	if (client)
		delete client;
	if (server)
		delete server;
}

bool FullNode::addNeighbour(unsigned int ID_, std::string IP_, unsigned int port_)
{
	//Nodo Full puede ser vecino con cualquier otro tipo de nodo.
	if (port_ < 0)
		return false;
	else {
		neighbours[ID_] = { IP_, port_};
		return true;
	}
}

/************************************************************************************************
*					                          MENSAJES											*
*																								*
*************************************************************************************************/

//POST Block
//Recibe ID del vecino, e ID del bloque a enviar
//Genera un JSON con los datos del ID del bloque (falta terminar esa funci�n) para luego adjuntarlo como header del mensaje Post
//S�lo configura el mensaje, la idea ser�a llamar a perform request (del nodo no del cliente) una vez seteado (por ah� desde el m�todo performRequest de cada nodo)
bool FullNode::POSTBlock(unsigned int neighbourID, std::string& blockId)
{
	if (neighbours.find(neighbourID) != neighbours.end())
	{
		if (state == FREE)
		{
			state = CLIENT;
			json block = createJSONBlock(blockId);
			client->setIP(neighbours[neighbourID].IP);
			client->setPort(neighbours[neighbourID].port);
			client->usePOSTmethod("/eda_coin/send_block", block);
			//client->performRequest(); //S�lo ejecuta una vuelta de multiHandle. Para continuar us�ndolo se debe llamar a la funci�n performRequest
			return true;
		}
		else return false;
	}	
	else return false;
}

/*POST Transaction
Recibe el ID del vecino, y la transacci�n a enviar.
Convierte la transacci�n en un JSON para luego adjuntarla como header del mensaje.
Igual al caso anterior, para terminar de ejecutar llamar a performRequest del nodo (NO de client!!).*/
bool FullNode::POSTTransaction(unsigned int neighbourID, Transaction Tx_)
{
	if (neighbours.find(neighbourID) != neighbours.end())
	{
		if (state == FREE)
		{
			state = CLIENT;
			json jsonTx = createJSONTx(Tx_);
			client->setIP(neighbours[neighbourID].IP);
			client->setPort(neighbours[neighbourID].port);
			client->usePOSTmethod("/eda_coin/send_tx", jsonTx);
			//client->performRequest();
			return true;
		}
		else return false;
	}
	else return false;
}

//POST Merkleblock
//Recibe el ID del vecino
//Para terminar de ejecutar usar performRequest del nodo (NO de client!!)
bool FullNode::POSTMerkleBlock(unsigned int neighbourID, std::string BlockID_, std::string TxID)
{
	if (neighbours.find(neighbourID) != neighbours.end())
	{
		if (state == FREE)
		{
			state = CLIENT;
			json jsonMerkleBlock = createJSONMerkleBlock(BlockID_, TxID);
			client->setIP(neighbours[neighbourID].IP);
			client->setPort(neighbours[neighbourID].port);
			client->usePOSTmethod("/eda_coin/send_merkle_block", jsonMerkleBlock);
			//client->performRequest();
			return true;
		}
		else return false;
	}
	else return false;
}

bool FullNode::GETBlocks(unsigned int neighbourID, std::string& blockID_, unsigned int count)
{
	if (neighbours.find(neighbourID) != neighbours.end())
	{
		if (state == FREE)
		{
			state = CLIENT;
			client->setIP(neighbours[neighbourID].IP);
			client->setPort(neighbours[neighbourID].port);
			client->useGETmethod("/eda_coin/get_blocks?block_id=" + blockID_ + "&count=" + to_string(count));
			return true;
		}
		else return false;
	}
	else return false;
}

/************************************************************************************************
*					                         Respuestas											*
*																								*
*************************************************************************************************/

//Respuesta a los mensajes del tipo POST.

std::string FullNode::POSTreply(std::string &receivedRequest, unsigned int clientPort_)
{
	json response;
	response["status"] = "true";
	response["result"] = NULL;
	receivedMessage = clientPort_;
	
	//Si se trata de un POSTblock guarda el block enviado
	if (receivedRequest.find("send_block") != std::string::npos)
	{


	}

	//Si se trata de un POSTtransaction
	else if (receivedRequest.find("send_tx") != std::string::npos)
	{
	}

	//Si se trata de un POSTfilter
	else if (receivedRequest.find("send_filter") != std::string::npos)
	{

	}

	/*return "HTTP/1.1 200 OK\r\nDate:" + makeDaytimeString(0) + "Location: " + "eda_coins" + "\r\nCache-Control: max-age=30\r\nExpires:" +
		makeDaytimeString(30) + "Content-Length:" + std::to_string(response.dump().length()) +
		"\r\nContent-Type: text/html; charset=iso-8859-1\r\n\r\n" + response.dump();*/
}

std::string FullNode::GETreply(std::string &receivedRequest, unsigned int clientPort_)
{
	json response;
	json tempresponse;
	response["status"] = "true";
	receivedMessage = clientPort_; //Indica de qui�n recib� el mensaje.

	if ((receivedRequest.find("send_block") != std::string::npos) || (receivedRequest.find("send_block_header") != std::string::npos))
	{
		unsigned int idPosition = receivedRequest.find("block_id=");
		unsigned int countPosition = receivedRequest.find("count=");

		if (idPosition != std::string::npos && countPosition != std::string::npos)
		{
			//Parseo id y count;
			std::string ID_ = receivedRequest.substr(idPosition + 9, receivedRequest.find_last_of("&") - idPosition - 9);
			std::string tempcount = receivedRequest.substr(countPosition + 6, receivedRequest.find("HTTP") - countPosition - 6);
			unsigned int index;
			unsigned int count = std::stoi(tempcount);
			//Recupero el �ndice al que apunta blockid
			for (int i = 0; i < NodeBlockchain.getBlocksSize(); i++)
			{
				if (NodeBlockchain.getBlocksArr()[i].getBlockID() == ID_) {
					index = i;
				}
			}
			//Verifico que no sea el �ltimo bloque
			if ((++index) > (NodeBlockchain.getBlocksSize() - 1))
			{
				//Error de contenido
				response["status"] = false;
				response["result"] = 2;
			}
			else
			{
				//Busco los bloques o headers de bloques solicitados.
				while (count && (index < NodeBlockchain.getBlocksSize()))
				{
					//Si se buscan bloques
					if (receivedRequest.find("send_block") != std::string::npos)
					{
						tempresponse.push_back(createJSONBlock(NodeBlockchain.getBlocksArr()[count].getBlockID()));
					}
					//Si se buscan headers
					if (receivedRequest.find("send_block_header") != std::string::npos)
					{
						tempresponse.push_back(createJSONHeader(NodeBlockchain.getBlocksArr()[count].getBlockID()));
					}
					count--;
				}
				//Adjunto la respuesta
				response["result"] = tempresponse;
			}
		}
		else
		{
			//Error de formato
			response["result"] = false;
			response["result"] = 1;
		}

	}
	else
	{
		//Error de contenido.
		response["status"] = false;
		response["result"] = 2;
	}

	/*return "HTTP/1.1 200 OK\r\nDate:" + makeDaytimeString(0) + "Location: " + "eda_coins" + "\r\nCache-Control: max-age=30\r\nExpires:" +
		makeDaytimeString(30) + "Content-Length:" + std::to_string(response.dump().length()) +
		"\r\nContent-Type: " + "text/html" + "; charset=iso-8859-1\r\n\r\n" + response.dump();*/
}


/************************************************************************************************
*					               GENERADORES DE JSON											*
*																								*
*************************************************************************************************/

//Genera un JSON del bloque de la blockchain que coincida con BlockId
json FullNode::createJSONBlock(std::string & BlockId)
{
	json jsonblock;
	Block block;
	for (int i = 0; i < NodeBlockchain.getBlocksSize(); i++)
	{
		if (NodeBlockchain.getBlocksArr()[i].getBlockID() == BlockId) {
			block = NodeBlockchain.getBlocksArr()[i];
			break;
		}
	}
	jsonblock["blockid"] = block.getBlockID();
	jsonblock["height"] = block.getHeight();
	jsonblock["merkleroot"] = block.getMerkleRoot();
	jsonblock["nTx"] = block.getNtx();
	jsonblock["nonce"] = block.getNonce();
	jsonblock["previousblockid"] = block.getPrevBlovkID();

	auto tx = json::array();
	for (auto tx_ = 0; tx_ < block.getNtx(); tx_++)
	{
		tx += createJSONTx(block.getTxVector()[tx_]);
	}
	jsonblock["tx"] = tx;
	return jsonblock;
}

//Genera el JSON de una transacci�n.
json FullNode::createJSONTx(Transaction Tx_)
{
	json jsonTx;
	jsonTx["nTxin"] = Tx_.nTxin;
	jsonTx["nTxout"] = Tx_.nTxout;
	jsonTx["txid"] = Tx_.txID;

	auto vin = json::array();	//Cargo el JSON de Vin dentro del JSON de transacciones.
	for (auto vin_ = 0; vin_ < Tx_.nTxin; vin_++)
	{
		vin.push_back(json::object({ {"txid",Tx_.vIn[vin_].txID}, {"outputIndex",Tx_.vIn[vin_].outputIndex}, {"signature",Tx_.vIn[vin_].signature}, {"blockid", Tx_.vIn[vin_].LilblockID} }));
	}
	jsonTx["vin"] = vin;

	auto vout = json::array(); //Cargo el JSON de Vout dentro del JSON de transacciones.
	for (auto vout_ = 0; vout_ < Tx_.nTxout; vout_++)
	{
		vout.push_back(json::object({ { "amount",Tx_.vOut[vout_].amount },{ "publicid", Tx_.vOut[vout_].publicID} }));
	}
	jsonTx["vout"] = vout;

	return jsonTx;
}

//En esta fase uso lo que dice la gu�a, creo que no hay que generar el merkle path
//Cargo con datos del primer bloque del arreglo.
//Para fases futuros hay que agregar en Block.h una funci�n que recupere el MerklePath
//Genera el JSON de un Merkle Block.
json FullNode::createJSONMerkleBlock(std::string BlockID_, std::string TxID)
{
	json MerkleBlock;
	Block block;
	Transaction tx;
	for (int i = 0; i < NodeBlockchain.getBlocksSize(); i++)
	{
		if (NodeBlockchain.getBlocksArr()[i].getBlockID() == BlockID_) {
			block = NodeBlockchain.getBlocksArr()[i];
			break;
		}
	}
	MerkleBlock["blockid"] = block.getBlockID();
	for (int i = 0; i < block.getTxVector().size(); i++)
	{
		if (block.getTxVector()[i].txID == TxID)
		{
			tx = block.getTxVector()[i];
			break;
		}
	}
	MerkleBlock["tx"] = createJSONTx(tx);
	MerkleBlock["txPos"] = 1;
	MerkleBlock["merklePath"] = block.getMerklePath(tx);
	/*json path = json::array();
	MerkleBlock["blockid"] = NodeBlockchain.getBlocksArr()[0].getBlockID();
	MerkleBlock["tx"] = createJSONTx(NodeBlockchain.getBlocksArr()[0].getTxVector()[0]);
	MerkleBlock["txPos"] = 1;
	for (int i = 0; i < NodeBlockchain.getBlocksArr()[0].getTxVector()[0].nTxin; i++)
	{
		path.push_back(json::object({ {"ID","1234"} }));
	}
	MerkleBlock["merklePath"] = path;
	return MerkleBlock;*/
	return MerkleBlock;
}

json FullNode::createJSONHeader(std::string BlockID_)
{
	json jsonHeader;
	Block block;
	for (int i = 0; i < NodeBlockchain.getBlocksSize(); i++)
	{
		if (NodeBlockchain.getBlocksArr()[i].getBlockID() == BlockID_) {
			block = NodeBlockchain.getBlocksArr()[i];
			break;
		}
	}
	jsonHeader["blockid"] = block.getBlockID();
	jsonHeader["height"] = block.getHeight();
	jsonHeader["merkleroot"] = block.getMerkleRoot();
	jsonHeader["nTx"] = block.getNtx();
	jsonHeader["nonce"] = block.getNonce();
	jsonHeader["previousblockid"] = block.getPrevBlovkID();

	return jsonHeader;
}


