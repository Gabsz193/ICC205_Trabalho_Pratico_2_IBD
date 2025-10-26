FROM gcc:12.3 AS builder

WORKDIR /app

COPY Makefile .
COPY src src

RUN make all

FROM debian:bookworm-slim AS runner

WORKDIR /app

COPY --from=builder /app/bin /app

ENTRYPOINT ["/app/"]

CMD ["/bin/bash", "-c", "echo 'Erro: Especifique o programa para rodar (upload, findrec, seek1, seek2)'"]