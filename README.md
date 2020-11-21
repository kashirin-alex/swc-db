# **SWC-DB©** &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;   ![SWC-DB](docs/logo-small.png)
## SWC-DB© (_Super Wide Column Database_) - High Performance Scalable Database



* ##### [CHANGELOG](https://github.com/kashirin-alex/swc-db/blob/master/CHANGELOG.md) _- start 'watching' the master branch to receive updates_

* ##### TODOS & WANTS
    * A Delete Query on matching cells ```delete [some-like select 'where_clause' syntax without incompatible flags]``` and returning the number of deleted cells - to let the Delete-Action to be done in one client-request an additional-option of instead (1)select + (2)apply DELETE flag + (3)send cells for update.
    * A Merge of empty Range to it's left(prior) sibling.
    * Persistent Storage Encryption - files: Fragments, CellStores - considerations crypted-data-chuncks with AES and corresponding tags & tokens encypted with a designated/configured RSA-key at header-level - to let Secure Transactions even if a File-System cannot have encryption support.
    * A fully-functioning JDBC within the possibly applicable-features - to let the development of [DBeaver SWC-DB plugin](https://github.com/dbeaver/dbeaver/issues/9034) and adding administration support with a GUI feature.
    * Services & Components statistics, an Updater(client::Query::Update) on each SWC-DB Program(Manager,Ranger,FsBroker,ThriftBroker) that writes metrics samples to the "SYS_STATS" column.
    * An Additional Column-Key-Sequence "MIXED", each fraction to have it's own Sequence type - limitation/requirements the key-fractions need to have a co-sequential Sequence Type.
    * a client::Query::ML::Train and a client::Query::ML::Predict with Serialized TensorFlow-Context(OpKernelContext) in the DB::Specs::Scan/CellsInterval - requires support of none-files based TF-OpKernelContext Slices and instead to work with a Buffer-Based(on req.) or/and SWC-DB select & update query.





### DOCUMENTATIONS

_All the documentations are based on the ./docs/_.

* ###### [SWC-DB Website - https://www.swcdb.org](https://www.swcdb.org) - plus the origins of the [Additional Documentations](https://www.swcdb.org/additional-docs/)

* ###### [SWC-DB GitHub Pages - https://kashirin-alex.github.io/swc-db/](https://kashirin-alex.github.io/swc-db/)

* ###### [SWC-DB Read The Docs - https://swc-db.readthedocs.io/](https://swc-db.readthedocs.io/)

* ###### [Browse the docs/](https://github.com/kashirin-alex/swc-db/blob/master/docs/)





### SUPPORT & DISCUSSIONS
* ###### Google Group is available at [groups.google.com/forum/#!forum/swc-db](https://groups.google.com/forum/#!forum/swc-db) for open discussions and help on SWC-DB





### ISSUES
* ###### open an issue at [github.com/kashirin-alex/swc-db/issues](https://github.com/kashirin-alex/swc-db/issues) in case there is an issue(bug/feature) that can be fully described.





### COMMERCIAL SUPPORT
* ###### Post a Job & Invite on [Upwork Profile](https://www.upwork.com/o/profiles/users/~016a24b743cc810aea/?s=1031626811434844160)
* ###### Contact for a Proposal Directly via kashirin.alex@gmail.com





***


### LICENSE

    SWC-DB© Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>
    SWC-DB is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, GPLv3 version.

    SWC-DB© is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. 
    If not, see <https://github.com/kashirin-alex/swc-db/blob/master/LICENSE>.


##### [COPYING NOTICE](COPYING.md)

If you intend to distribute your software implementation-bundeled with SWC-DB under the term "covered work" of GPLv3 License.

For compliance to Section 10. Automatic Licensing of Downstream Recipients of GPLv3 License.

The notice below of license shall be used:

    SWC-DB© - Super Wide Column Database
    Copyright since 2019 Alex Kashirin <kashirin.alex@gmail.com>

    This product includes software developed at
      <https://github.com/kashirin-alex/swc-db>

    License details at
      <https://github.com/kashirin-alex/swc-db/#license>



##### [THIRD-PARTY LICENSE NOTICE](NOTICE.md)

    SWC-DB implement third-party sources and binaries in dynamic or static form.

    The Third-Party license permissions, limitations and conditions 
    vary between the types of implementation depends on a SWC-DB release. 
    
    Whereas to this SWC-DB license under GPLv3 license 
    the applied implication is the 'static' form,
    While 'dynamic' applied to 
     libpam_swcdb_max_retries, libswcdb_fs_hadoop_jvm, libswcdb_fs_ceph.

    The Third-Party License notices available at: 
     <https://github.com/kashirin-alex/swc-db/blob/master/NOTICE.md>



#### Non-Commercial License

    No other License available

#### Commercial License

    For a Commercial Purpose License in-case activities limited by GPLv3.
    You are welcome to inquiry with requirements via kashirin.alex@gmail.com

***
