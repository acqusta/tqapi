package xtz.tquant.api.scala.impl

import java.lang.reflect.{ParameterizedType, Type}

import com.fasterxml.jackson.annotation.JsonInclude.Include
import com.fasterxml.jackson.core.`type`.TypeReference
import com.fasterxml.jackson.databind.{DeserializationFeature, JsonNode, ObjectMapper}
import com.fasterxml.jackson.module.scala.DefaultScalaModule
import xtz.tquant.api.java.JsonRpc

object JsonHelper {

    val mapper = new ObjectMapper()
    mapper.setSerializationInclusion(Include.NON_NULL)
    mapper.registerModule(DefaultScalaModule)
    mapper.configure(DeserializationFeature.FAIL_ON_UNKNOWN_PROPERTIES, false)


    def serialize(value: Any): String = mapper.writeValueAsString(value)

    def deserialize[T: Manifest](value: String) : T = mapper.readValue(value, typeReference[T])

    def toJson(value: String): JsonNode = mapper.readTree(value)

    private [this] def typeReference[T: Manifest] = new TypeReference[T] {
        override def getType : Type = typeFromManifest(manifest[T])
    }

    private[this] def typeFromManifest(m: Manifest[_]): Type = {
        if (m.typeArguments.isEmpty) {
            m.runtimeClass
        } else{
            new ParameterizedType {
                override def getRawType : Type = m.runtimeClass
                override def getActualTypeArguments : Array[Type] = m.typeArguments.map(typeFromManifest).toArray
                override def getOwnerType : Type = null
            }
        }
    }


    def convert[T: Manifest] (obj: Any): T = {
        mapper.convertValue(obj, typeReference[T])
    }


    def erroToString(error: JsonRpc.JsonRpcError) : String = {
        if (error != null) {
            if (error.message != null )
                error.code.toString + "," + error.message
            else
                error.code.toString + ","
        } else {
            ","
        }
    }

    def extractResult[T: Manifest](cr: JsonRpc.JsonRpcCallResult,  errValue: T = null): (T, String) = {
        try {
            val value =
                if (cr.result != null ) {
                    JsonHelper.convert[T](cr.result)
                }
                else
                    errValue

            val message =
                if ( cr.error != null )
                    erroToString(cr.error)
                else
                    null

            (value, message)


        } catch {
            case e: Throwable =>
                e.printStackTrace()
                (errValue, e.getMessage)
        }
    }

    def extractResultMapList(cr: JsonRpc.JsonRpcCallResult): (Map[String, List[_]], String) = {
        try {
            val value =
                if (cr.result != null )
                    cr.result.asInstanceOf[Map[String, List[_]]]
                else
                    null

            val message =
                if ( cr.error != null )
                    erroToString(cr.error)
                else
                    null

            (value, message)


        } catch {
            case e: Throwable =>
                e.printStackTrace()
                (null, e.getMessage)
        }
    }

}