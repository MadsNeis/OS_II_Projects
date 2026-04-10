### README Answers

- `reader_count` must be atomic because multiple reader threads can be inside the critical section at simultaneously, so they can increment and decrement it. Without atomics, the read and writes would be a data race. 
- `writer_count` doesn't need to be atomic because the write lock makes sure only one writer is ever in the critical section at a time. This way, there's no data race. 