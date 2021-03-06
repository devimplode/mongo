// @file balancer_policy.h

/**
*    Copyright (C) 2010 10gen Inc.
*
*    This program is free software: you can redistribute it and/or  modify
*    it under the terms of the GNU Affero General Public License, version 3,
*    as published by the Free Software Foundation.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU Affero General Public License for more details.
*
*    You should have received a copy of the GNU Affero General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef S_BALANCER_POLICY_HEADER
#define S_BALANCER_POLICY_HEADER

#include "mongo/db/jsobj.h"

namespace mongo {

    class BalancerPolicy {
    public:
        struct MigrateInfo;

        struct ChunkInfo {
            const BSONObj min;
            const BSONObj max;
            
            ChunkInfo( const BSONObj& a_min, const BSONObj& a_max ) 
                : min( a_min ), max( a_max ){}

            ChunkInfo( const BSONObj& chunk ) 
                : min( chunk["min"].Obj() ), max( chunk["max"].Obj() ) {
            }

            string toString() const;
        };

        class ShardInfo {
        public:
            ShardInfo();
            ShardInfo( long long maxSize, long long currSize, bool draining, bool opsQueued );
            
            /**
             * @return true if a shard cannot receive any new chunks bacause it reache 'shardLimits'.
             * Expects the optional fields "maxSize", can in size in MB, and "usedSize", currently used size
             * in MB, on 'shardLimits'.
             */
            bool isSizeMaxed() const;

            /**
             * @return true if 'shardLimist' contains a field "draining". Expects the optional field
             * "isDraining" on 'shrdLimits'.
             */
            bool isDraining() const { return _draining; }

            /**
             * @return true if a shard currently has operations in any of its writeback queues
             */
            bool hasOpsQueued() const { return _hasOpsQueued; }

            string toString() const;

        private:
            long long _maxSize;
            long long _currSize;
            bool _draining;
            bool _hasOpsQueued;
        };
        
        /**
         * Returns a suggested chunk to move whithin a collection's shards, given information about
         * space usage and number of chunks for that collection. If the policy doesn't recommend
         * moving, it returns NULL.
         *
         * @param ns is the collections namepace.
         * @param shardToChunksMap is a map from shardId to chunks that live there. A chunk's format
         * is { }.
         * @param balancedLastTime is the number of chunks effectively moved in the last round.
         * @returns NULL or MigrateInfo of the best move to make towards balacing the collection.
         */
        typedef map< string,ShardInfo > ShardInfoMap;
        typedef map< string,vector<BSONObj> > ShardToChunksMap;
        static MigrateInfo* balance( const string& ns, 
                                   const ShardInfoMap& shardInfo,
                                   const ShardToChunksMap& shardToChunksMap, 
                                   int balancedLastTime );

        // below exposed for testing purposes only -- treat it as private --

        static BSONObj pickChunk( const vector<BSONObj>& from, const vector<BSONObj>& to );

    };

    struct BalancerPolicy::MigrateInfo {
        const string ns;
        const string to;
        const string from;
        const ChunkInfo chunk;

        MigrateInfo( const string& a_ns , const string& a_to , const string& a_from , const BSONObj& a_chunk )
            : ns( a_ns ) , to( a_to ) , from( a_from ), chunk( a_chunk ) {}


    };

}  // namespace mongo

#endif  // S_BALANCER_POLICY_HEADER
